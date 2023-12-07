#ifndef OPERATEDB_H
#define OPERATEDB_H

#include <QObject>
#include <QSqlDatabase> //连接数据库
#include <QSqlQuery> //查询数据库
#include <QStringList> //字符串列表

//数据库操作类
class OperateDB : public QObject
{
    Q_OBJECT
public:
    explicit OperateDB(QObject *parent = nullptr);
    static OperateDB& getInstance();
    void init();
    ~OperateDB();

    bool handleRegist(const char *name,const char *pwd);
    bool handleLogin(const char *name,const char *pwd);
    void handleOffline(const char *name);
    QStringList handleAllOnline();
    QStringList handleSearchUser(const char* name);

    int handleAddFriend(const char* friendName,const char* myName);
    void handleAddRelationship(const char* friendName,const char* myName);
    QStringList handldFlushFriend(const char* name);
    bool handleDeleteFriend(const char* friendName,const char *myName);
    QStringList handleGroupFriend(const char* name);
    bool handleLogoff(const char* name,const char *pwd);

signals:

public slots:
private:
    QSqlDatabase m_db;//连接数据库

};

#endif // OPERATEDB_H
