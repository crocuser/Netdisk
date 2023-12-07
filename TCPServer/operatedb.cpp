#include "operatedb.h"
#include <QMessageBox>
#include <QDebug>


OperateDB::OperateDB(QObject *parent): QObject{parent}
{
    m_db=QSqlDatabase::addDatabase("QSQLITE");//连接sqlite
}

OperateDB &OperateDB::getInstance()
{
    static OperateDB instance;
    return instance;//因为是静态的，每次返回同一个对象
}

void OperateDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("F:\\Qt\\qt_project\\TCPServer\\cloud.db");
    if(m_db.open())
    {
        //打开成功，查询测试一下
        QSqlQuery query;
        // query.exec("select * from userInfo"); //执行sql查询
        // while(query.next())//读每条数据
        // {
        //     QString data=QString("%1,%2,%3").arg(query.value(1).toString()).arg(query.value(2).toString()).arg(query.value(3).toString());
        //     qDebug()<<data;
        // }
        qDebug()<<"打开数据库成功";
    }
    else
    {
        // QMessageBox::critical(NULL,"打开数据库","打开数据库失败！");
        qDebug()<<"打开数据库失败！";
    }
}

OperateDB::~OperateDB()
{
    m_db.close();//关闭数据库
}


bool OperateDB::handleRegist(const char *name, const char *pwd)
{
    //考虑形参的有效性
    if(NULL==name||NULL==pwd)
    {
        qDebug()<<"name | pwd is null";
        return false;
    }
    //拼接sql语句
    QString sqlStatement=QString("insert into userInfo(name,pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    qDebug()<<sqlStatement;

    QSqlQuery query;
    return query.exec(sqlStatement);//直接返回执行结果（bool）

}

bool OperateDB::handleLogin(const char *name, const char *pwd)
{
    if(NULL==name||NULL==pwd)
    {
        qDebug()<<"name | pwd is null";
        return false;
    }
    QString sqlStatement=QString("select * from userInfo where name=\'%1\' and pwd=\'%2\' and online=0").arg(name).arg(pwd);
    qDebug()<<sqlStatement;

    QSqlQuery query;
    query.exec(sqlStatement);//直接返回执行结果（bool）
    // return query.next();//返回查询结果

    //更新用户的登录状态
    if(query.next())
    {
        QString sqlStatement=QString("update userInfo set online=1 where name=\'%1\' and pwd=\'%2\' ").arg(name).arg(pwd);
        qDebug()<<sqlStatement;

        QSqlQuery query;
        query.exec(sqlStatement);

        return true;
    }
    else
    {
        return false;
    }
}

void OperateDB::handleOffline(const char *name)
{
    //形参有效性
    if(NULL==name)
    {
        qDebug()<<"name is NULL";
        return;
    }
    //该用户状态设置为下线
    QString sqlStatement=QString("update userInfo set online=0 where name=\'%1\' ").arg(name);
    qDebug()<<sqlStatement;

    QSqlQuery query;
    query.exec(sqlStatement);

}

QStringList OperateDB::handleAllOnline()
{
    QString sqlStatement=QString("select name from userInfo where online=1");
    qDebug()<<sqlStatement;

    QSqlQuery query;
    query.exec(sqlStatement);

    QStringList nameList;
    nameList.clear();
    while(query.next())
    {
        nameList.append(query.value(0).toString());
    }
    return nameList;
}

QStringList OperateDB::handleSearchUser(const char *name)
{
    QStringList resultList={};
    if(NULL == name)
    {
        return resultList;
    }
    QString sqlStatement=QString("select name,online from userInfo where name like \'\%%1\%\' ").arg(name);//模糊查询，返回姓名和在线状态
    qDebug()<<sqlStatement;

    QSqlQuery query;
    query.exec(sqlStatement);
    while(query.next())
    {
        QString name = query.value(0).toString();
        QString online = query.value(1).toString();

        QString resultString = name + "," + online;
        resultList.append(resultString);

    }

    // 输出 QStringList 中的元素
    for (const QString &str : resultList) {
        qDebug() << str;
    }

    return resultList;

}

int OperateDB::handleAddFriend(const char *friendName, const char *myName)
{
    if(NULL==friendName||NULL==myName)  return -1;
    QString selectFridendId=QString("select id from userInfo where name=\'%1\'").arg(friendName);
    QString selectMyId=QString("select id from userInfo where name=\'%1\'").arg(myName);
    //双向查询
    QString sqlStatement=QString("select * from friend where (id=(%1) and friendId =(%2)) or (id=(%3) and friendId =(%4))").arg(selectMyId).arg(selectFridendId).arg(selectFridendId).arg(selectMyId);
    qDebug()<<sqlStatement;

    QSqlQuery query;
    query.exec(sqlStatement);
    if(query.next()) return 2;//双方已经是好友
    else
    {
        sqlStatement=QString("select online from userInfo where name=\'%1\'").arg(friendName);
        query.exec(sqlStatement);
        if(query.next())
        {
            int res=query.value(0).toInt();
            return res;//此人的状态
        }
        else
        {
            return -2;//没有这个人
        }
    }
}

void OperateDB::handleAddRelationship(const char *friendName, const char *myName)
{
    if(NULL==friendName||NULL==myName)  return;
    QString selectFridendId=QString("select id from userInfo where name=\'%1\'").arg(friendName);
    QString selectMyId=QString("select id from userInfo where name=\'%1\'").arg(myName);
    QString sqlStatement=QString("insert into friend (id,friendId) values ((%1),(%2))").arg(selectMyId).arg(selectFridendId);
    qDebug()<<sqlStatement;

    QSqlQuery query;
    query.exec(sqlStatement);//执行插入
}

QStringList OperateDB::handldFlushFriend(const char *name)//刷新好友
{
    QStringList strFriendList;
    strFriendList.clear();
    if(name==NULL) return strFriendList;//直接返回空的列表
    //查自己的id
    QString selectNameId=QString("select id from userInfo where name=\'%1\'").arg(name);
    //查朋友的id
    QString sqlStatement=QString("select id from friend where friendId in (%1)").arg(selectNameId);
    QString sqlStatement2=QString("select friendId from friend where id in (%1)").arg(selectNameId);
    //查朋友的名字和状态
    QString sqlStatement3=QString("select name,online from userInfo where id in (%1) or id in (%2)").arg(sqlStatement).arg(sqlStatement2);
    qDebug()<<sqlStatement3;

    QSqlQuery query;
    query.exec(sqlStatement3);//执行
    while(query.next())
    {
        QString name = query.value(0).toString();
        QString online = query.value(1).toString();

        strFriendList.append(name + "," + online);

    }
    // 输出 QStringList 中的元素
    for (const QString &str : strFriendList) {
        qDebug() << str;
    }

    return strFriendList;
}

bool OperateDB::handleDeleteFriend(const char *friendName, const char *myName)
{
    if(NULL==friendName||NULL==myName)  return false;

    QString selectFridendId=QString("select id from userInfo where name=\'%1\'").arg(friendName);
    QString selectMyId=QString("select id from userInfo where name=\'%1\'").arg(myName);
    QString sqlStatement=QString("delete from friend where (id in (%1) and friendId in (%2)) or (friendId in (%3) and id in (%4))").arg(selectMyId).arg(selectFridendId).arg(selectMyId).arg(selectFridendId);
    qDebug()<<sqlStatement;

    QSqlQuery query;
    return query.exec(sqlStatement);//执行删除

}

QStringList OperateDB::handleGroupFriend(const char *name)//查所有在线好友
{
    QStringList strFriendList;
    strFriendList.clear();
    if(name==NULL) return strFriendList;//直接返回空的列表
    //查自己的id
    QString selectNameId=QString("select id from userInfo where name=\'%1\'").arg(name);
    //查朋友的id
    QString sqlStatement=QString("select id from friend where friendId in (%1)").arg(selectNameId);
    QString sqlStatement2=QString("select friendId from friend where id in (%1) ").arg(selectNameId);
    //查朋友的名字
    QString sqlStatement3=QString("select name from userInfo where (id in (%1)  and online =1) or (id in (%2) and online =1)").arg(sqlStatement).arg(sqlStatement2);
    qDebug()<<sqlStatement3;

    QSqlQuery query;
    query.exec(sqlStatement3);//执行
    while(query.next())
    {
        QString friendName = query.value(0).toString();
        strFriendList.append(friendName);
    }
    strFriendList.append(name);//群聊时，自己可以看到自己发的消息
    // 输出 QStringList 中的元素
    for (const QString &str : strFriendList) {
        qDebug() << str;
    }

    return strFriendList;
}

//注销操作
bool OperateDB::handleLogoff(const char *name, const char *pwd)
{
    //形参有效性
    if(NULL==name)
    {
        qDebug()<<"name is NULL";
        return false;
    }
    QString sqlStatement=QString("select id from userInfo where name=\'%1\' and pwd=\'%2\'").arg(name).arg(pwd);//查询将删除的人的id
    QSqlQuery query;
    query.exec(sqlStatement);//执行
    //判断此人是否存在
    if(!query.next())
    {
        return false;//此人不存在
    }
    int id=query.value(0).toInt();
    qDebug()<<id;
    //删除此人
    QString sqlStatement2=QString("delete from userInfo where (id =%1)").arg(id);
    QSqlQuery query2;

    //执行删除此人好友关系的操作
    QString sqlStatement3=QString("delete from friend where (id =%1) or (friendId=%2)").arg(id).arg(id);
    QSqlQuery query3;

    qDebug()<<query2.exec(sqlStatement2);
    qDebug()<<query3.exec(sqlStatement3);

    return query2.exec(sqlStatement2) && query3.exec(sqlStatement3);//执行
}
