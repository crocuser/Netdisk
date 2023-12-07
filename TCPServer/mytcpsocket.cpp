#include "mytcpsocket.h"
#include <QDebug>
#include "operatedb.h" //操作数据库

#include "mytcpserver.h"
#include <QDir> //目录操作
#include <QFileInfoList>//获取文件信息
#include <QtMath>//数学计算

MyTcpSocket::MyTcpSocket(QObject *parent): QTcpSocket{parent}
{
    //各自的socket自己产生的消息，自己处理
    // connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    connect(this, &QIODevice::readyRead, this, &MyTcpSocket::recvMsg);

    //一旦客户端发出disconnected信号，就触发clientOffline函数
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffline()));

    connect(m_pTimer,&QTimer::timeout,this,&MyTcpSocket::sendFileToClient);//上传文件内容
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

void MyTcpSocket::copyDir(QString strSrcDir, QString strDestDir)//拷贝目录
{
    QDir dir;
    dir.mkdir(strDestDir);

    dir.setPath(strSrcDir);
    QFileInfoList fileInfoList=dir.entryInfoList();
    for(int i=0;i<fileInfoList.size();i++)
    {
        if(fileInfoList[i].isFile())
        {
            QFile::copy(strSrcDir+'/'+fileInfoList[i].fileName(),strDestDir+'/'+fileInfoList[i].fileName());
        }
        else if(fileInfoList[i].isDir())
        {
            if(QString(".")==fileInfoList[i].fileName() || QString("..")==fileInfoList[i].fileName())
            {
                continue;
            }
            this->copyDir(strSrcDir+'/'+fileInfoList[i].fileName(),strDestDir+'/'+fileInfoList[i].fileName());//递归拷贝
        }
    }
}

void MyTcpSocket::recvMsg()
{
    if (this->m_bIsDataStream)//如果是数据流
    {
        this->recvDataStream();//调用数据流进行处理
        return;
    }
    qDebug()<<this->bytesAvailable();//返回当前设备中可以读取的字节数，用于测试
    uint uiPDULen=0;
    this->read((char*)&uiPDULen,sizeof(uint));
    //实际消息长度
    uint uiMsgLen=uiPDULen-sizeof(PDU);
    PDU *pdu=mkPDU(uiMsgLen); //制造pdu
    this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));

    char caName[32]={'\0'};//用户名，或好友的名字
    char caPwd[32]={'\0'};//密码,或我自己的名字,或目录名
    strncpy(caName,pdu->caData,32);
    strncpy(caPwd,pdu->caData+32,32);

    //不同的消息类型，对应不同的处理方式
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_REGIST_REQUEST://注册请求
    {
        bool res=OperateDB::getInstance().handleRegist(caName,caPwd);
        PDU *resPdu=mkPDU(0);
        resPdu->uiMsgType=ENUM_MSG_REGIST_RESPOND;//消息类型：注册回复
        if(res)
        {
            strcpy(resPdu->caData,REGIST_OK);
            //在注册成功后，创建用户的根目录
            QDir dir;
            bool result=dir.mkpath(QString("../user/%1/share").arg(caName));//多级目录
            qDebug()<<"创建用户根目录"<<result;
        }
        else
        {
            strcpy(resPdu->caData,REGIST_FAILED);
        }
        //和客户端发送类似
        this->write((char*)resPdu,resPdu->uiPDULen);
        free(resPdu);
        resPdu=NULL;
        break;
    }
    case ENUM_MSG_LOGIN_REQUEST://登录请求
    {
        bool res=OperateDB::getInstance().handleLogin(caName,caPwd);
        PDU *resPdu=mkPDU(0);//这重复代码有点多啊
        resPdu->uiMsgType=ENUM_MSG_LOGIN_RESPOND;//消息类型：注册回复
        if(res)
        {
            strcpy(resPdu->caData,LOGIN_OK);
            m_strName=caName;//登录成功后，记录用户名
            //在登录成功后，记录用户根目录

        }
        else
        {
            strcpy(resPdu->caData,LOGIN_FAILED);
        }
        //和客户端发送类似
        this->write((char*)resPdu,resPdu->uiPDULen);
        free(resPdu);
        resPdu=NULL;
        break;
    }
    case ENUM_MSG_ALL_ONLINE_REQUEST://显示所有在线用户请求
    {
        QStringList nameList=OperateDB::getInstance().handleAllOnline();
        uint uiMsgLen=nameList.size()*32;//消息长度
        PDU *respdu=mkPDU(uiMsgLen);
        respdu->uiMsgType=ENUM_MSG_ALL_ONLINE_RESPOND;
        for(int i=0;i<nameList.size();++i)
        {
            //每次偏移32位，char占1位，强转为char*型
            memcpy((char*)(respdu->caMsg)+i*32,nameList.at(i).toStdString().c_str(),nameList.at(i).size());

        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_SEARCH_USERS_REQUEST://搜索用户请求
    {
        QStringList resultList=OperateDB::getInstance().handleSearchUser(caName);
        uint uiMsgLen=resultList.size()*32;
        PDU *respdu=mkPDU(uiMsgLen);
        respdu->uiMsgType=ENUM_MSG_SEARCH_USERS_RESPOND;

        if(uiMsgLen==0)
        {
            strcpy((char*)respdu->caMsg,SEARCH_USER_NO);
        }
        else
        {
            for(int i=0;i<resultList.size();++i)
            {
                memcpy((char*)respdu->caMsg+i*32,resultList.at(i).toStdString().c_str(),resultList.at(i).size());
            }
        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_ADD_FRIEND_REQUEST://添加好友请求
    {
        //判断是不是好友

        //对方在不在线

        //对方同意还是拒绝
        int res=OperateDB::getInstance().handleAddFriend(caName,caPwd);
        PDU  *respdu=mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_ADD_FRIEND_RESPOND;
        if(-1==res)
        {
            //发生未知错误，没有得到friend和myname
            strcpy(respdu->caData,UNKNOW_ERROR);
        }
        else if (2==res)
        {
            //已经是好友
            strcpy(respdu->caData,EXISTED_FRIEND);
        }
        else if(-2==res)
        {
            //没有这个人
            strcpy(respdu->caData,ADD_FRIEND_NOEXIST);
        }
        else if(0==res)
        {
            //此人不在线
            strcpy(respdu->caData,ADD_FRIEND_OFFLINE);
        }
        else if(1==res)
        {
            //在线可添加为好友，获得对方的socket转发消息
            MyTCPServer::getInstance().resend(caName,pdu);
            break;
        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_ADD_FRIEND_AGREE://同意了添加好友的请求
    {
        //修改数据库
        //向发送方反馈消息
        OperateDB::getInstance().handleAddRelationship(caName,caPwd);
        // PDU  *respdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_ADD_FRIEND_AGREE;
        MyTCPServer::getInstance().resend(caPwd,pdu);//转发给我
        break;
    }
    case ENUM_MSG_ADD_FRIEND_REFUSE://拒绝了添加好友的请求
    {
        //直接向发送方反馈消息
        pdu->uiMsgType=ENUM_MSG_ADD_FRIEND_REFUSE;
        MyTCPServer::getInstance().resend(caPwd,pdu);//转发给我
        break;
    }
    case ENUM_MSG_FLUSH_FRIEND_REQUEST://刷新好友请求
    {
        QStringList friendList=OperateDB::getInstance().handldFlushFriend(caName);
        uint uiMsgLen=friendList.size()*32;
        PDU *respdu=mkPDU(uiMsgLen);
        respdu->uiMsgType=ENUM_MSG_FLUSH_FRIEND_RESPOND;
        for(int i=0;i<friendList.size();++i)
        {
            memcpy((char*)(respdu->caMsg)+i*32,friendList.at(i).toStdString().c_str(),friendList.at(i).size());//拷贝
        }
        this->write((char*)respdu,respdu->uiPDULen);

        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_DELETE_FRIEND_REQUEST://删除好友请求
    {
        OperateDB::getInstance().handleDeleteFriend(caName,caPwd);

        //通知双方
        PDU *respdu=mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_DELETE_FRIEND_RESPOND;
        strcpy(respdu->caData,DELETE_FRIEND_OK);
        this->write((char*)respdu,respdu->uiPDULen);
        //通知被删除者

        strcpy(respdu->caData,QString("你被 %1 删除好友了呢").arg(caPwd).toStdString().c_str());
        MyTCPServer::getInstance().resend(caName,respdu);

        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_PRIVATE_CHAT_REQUEST://私聊请求
    {
        MyTCPServer::getInstance().resend(caName,pdu);//转发给对方
        MyTCPServer::getInstance().resend(caPwd,pdu);//给自己也发一份
        qDebug()<<caName;
        break;
    }
    case ENUM_MSG_GROUP_CHAT_REQUEST://群聊请求
    {
        QStringList onlineFriendList=OperateDB::getInstance().handleGroupFriend(caName);
        for(int i=0;i<onlineFriendList.size();++i)
        {
            MyTCPServer::getInstance().resend(onlineFriendList.at(i).toStdString().c_str(),pdu);//逐个分发
        }
        break;
    }
    case ENUM_MSG_CREATE_DIR_REQUEST://新建目录请求
    {
        QDir dir;
        QString strCurrentPath=QString((char*)(pdu->caMsg));
        QString dirName=strCurrentPath+"/"+caPwd;
        PDU *respdu=mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_CREATE_DIR_RESPOND;//新建目录回复
        if(dir.exists(dirName))
        {
            //需要新建的目录已经存在，新建失败
            strcpy(respdu->caData,DIR_EXIST);
        }
        else
        {
            bool result=dir.mkdir(dirName);
            qDebug()<<dirName<<"创建:"<<result;
            if(result)
            {
                strcpy(respdu->caData,DIR_CREATE_OK);
            }
            else
            {
                strcpy(respdu->caData,DIR_CREATE_FAILED);
            }
        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_FLUSH_DIR_REQUEST://刷新目录请求
    {
        char *pCurrentPaht=new char[pdu->uiMsgLen];//当前路径
        memcpy(pCurrentPaht,pdu->caMsg,pdu->uiMsgLen);

        QDir dir(pCurrentPaht);
        qDebug()<<pCurrentPaht;
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot); // 设置过滤器，显示目录和文件，不显示.和..
        QFileInfoList fileInfoList=dir.entryInfoList();//获取指定目录下的文件信息列表

        PDU *respdu=mkPDU(fileInfoList.size()*128);
        respdu->uiMsgType=ENUM_MSG_FLUSH_DIR_RESPOND;//刷新目录回复
        strcpy(respdu->caData,(char*)pdu->caMsg);
        QString message;
        int i=0;
        //单位换算：将b转换为kb
        double multiple=qPow(2.0,10);
        // 使用 foreach 循环遍历文件信息列表
        foreach (const QFileInfo &fileInfo, fileInfoList)
        {
            if(fileInfo.isDir())
            {
                // qDebug() << fileInfo.fileName()<<"\t文件类型："<<"[DIR]"<<"创建时间："<<fileInfo.birthTime().toString("yyyy-MM-dd hh:mm:ss");
                message=fileInfo.fileName()+"\t[DIR]\t"+fileInfo.birthTime().toString("yyyy-MM-dd hh:mm:ss");
            }
            else
            {
                double resultSize=fileInfo.size()/multiple;//单位转换
                QString precisionThreeSize=QString::number(resultSize,'f',3);//三位精度
                // qDebug() << fileInfo.fileName()<<"\t文件类型："<<fileInfo.suffix()<<"创建时间："<<fileInfo.birthTime().toString("yyyy-MM-dd hh:mm:ss")<<"文件大小："+precisionThreeSize<<"KB";
                message=fileInfo.fileName()+"\t"+fileInfo.suffix()+"\t"+fileInfo.birthTime().toString("yyyy-MM-dd hh:mm:ss")+"\t"+precisionThreeSize+" KB";
            }
            QByteArray byteArray = message.toUtf8();//将 QString 转换为 UTF-8 编码格式,编码的问题，导致传输数据不完整，显示不全
            memcpy((char*)(respdu->caMsg)+i*128,byteArray,byteArray.size());
            qDebug()<<byteArray;
            ++i;
        }
        qDebug()<<QString((char*)(respdu->caMsg)+128*5);
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_DELETE_FILE_REQUEST://删除文件或目录请求
    {
        char *pPath=new char[pdu->uiMsgLen];
        memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);

        QString strFile=QString(pPath)+"/"+caName;
        qDebug()<<strFile;
        QFileInfo fileInfo=QFileInfo(strFile);
        bool res=false;
        if(fileInfo.isDir())//判断是否是目录
        {
            QDir dir(strFile);
            res=dir.removeRecursively();//递归删除目录
        }
        else
        {
            QFile file(strFile);
            res=file.remove();
        }
        PDU *respdu=mkPDU(64+strFile.size()+1);
        respdu->uiMsgType=ENUM_MSG_DELETE_FILE_RESPOND;
        if(res)
        {
            strcpy((char*)respdu->caMsg,QString(strFile+" "+DELETE_FILE_OK).toUtf8().cbegin());//删除成功
        }
        else
        {
            strcpy((char*)respdu->caMsg,QString(strFile+" "+DELETE_FILE_FAILED).toUtf8().cbegin());//删除失败
        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_RENAME_FILE_REQUEST://重命名文件或目录请求
    {
        char *pPath=new char[pdu->uiMsgLen];//父目录路径
        memcpy(pPath,(char*)pdu->caMsg,pdu->uiMsgLen);

        QString strOldName=QString(pPath)+"/"+caName;//旧名字
        QString strNewName=QString(pPath)+"/"+caPwd;//新名字

        QFileInfo fileInfo=QFileInfo(strOldName);
        bool res=false;
        if(fileInfo.isDir())//判断是否是目录
        {
            QDir dir;
            res=dir.rename(strOldName,strNewName);
        }
        else
        {
            QFile file;
            res=file.rename(strOldName,strNewName);
        }
        qDebug()<<strNewName;
        PDU *respdu=mkPDU(128+strNewName.size()+1);
        respdu->uiMsgType=ENUM_MSG_RENAME_FILE_RESPAND;//重命名文件回复
        if(res)
        {
            strcpy((char*)respdu->caMsg,QString(strOldName+", "+RENAME_FILE_OK+", for "+strNewName).toUtf8().cbegin());//重命名成功
        }
        else
        {
            strcpy((char*)respdu->caMsg,QString(strOldName+", "+RENAME_FILE_FAILED).toUtf8().cbegin());//重命名失败
        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_CREATE_FILE_REQUEST://新建文件请求
    {
        QString strCurrentPath=QString((char*)(pdu->caMsg));
        QString strFilePath=strCurrentPath+"/"+caName;

        PDU *respdu=mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_CREATE_FILE_RESPAND;

        if(QFile::exists(strFilePath))
        {
            //文件已存在
            strcpy(respdu->caData,FILE_EXIST);
        }
        else
        {
            //文件不存在创建文件
            QFile file(strFilePath);
            if(file.open(QIODevice::ReadWrite | QIODevice::Truncate))
            {
                //文件创建成功
                file.close();//关闭文件
                strcpy(respdu->caData,FILE_CREATE_OK);
            }
            else
            {
                strcpy(respdu->caData,FILE_CREATE_FAILED);
            }
        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_UPLOAD_FILE_REQUEST://上传文件请求
    {
        // char caFileName[32]={'\0'};
        // qint64 fileSize=0;
        // sscanf(pdu->caData,"%s %11d",caFileName,&fileSize);//读出文件名和文件大小

        qint64 fileSize = QString(caPwd).toLongLong();
        qDebug()<<caName;

        QString strPath = QString("%1/%2").arg((char*)pdu->caMsg).arg(caName);
        qDebug()<<strPath;

        //新建一个空文件
        this->m_File.setFileName(strPath);
        //以只写的方式打开文件，若文件不存在，则会自动创建文件
        if(this->m_File.open(QIODevice::WriteOnly))
        {
            this->m_bIsDataStream=true;//接下来将传输二进制数据流
            this->m_iTotal=fileSize;
            this->m_iRecved=0;
            this->m_File.close();//关闭文件
        }

        break;
    }
    case ENUM_MSG_DOWNLOAD_FILE_REQUEST://下载文件请求
    {
        QString strPath = QString("%1/%2").arg((char*)pdu->caMsg).arg(caName);//请求下载的文件路径
        qDebug()<<strPath;

        QFileInfo fileInfo(strPath);
        qint64 fileSize=fileInfo.size();
        PDU *respdu=mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_DOWNLOAD_FILE_RESPAND;//下载文件回复
        sprintf(respdu->caData,"%s %11lld",caName,fileSize);

        this->m_File.setFileName(strPath);//打开服务器端的文件

        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;

        this->m_pTimer->start(1000);//给客户端回复后，启动计时器

        break;
    }
    case ENUM_MSG_SHARE_FILE_REQUEST://分享文件请求
    {
        char caSendName[32]={'\0'};
        int count=0;
        sscanf(pdu->caData,"%s %d",caSendName,&count);

        PDU *respdu=mkPDU(pdu->uiMsgLen-count*32);
        respdu->uiMsgType=ENUM_MSG_SHARE_FILE_NOTICE;//共享文件通知
        strcpy(respdu->caData,caSendName);//分享者
        memcpy((char*)respdu->caMsg,(char*)(pdu->caMsg)+count*32,pdu->uiMsgLen-count*32);//分享的文件

        char caRecvName[32]={'\0'};
        for(int i=0;i<count;i++)
        {
            memcpy(caRecvName,(char*)(pdu->caMsg)+i*32,32);
            MyTCPServer::getInstance().resend(caRecvName,respdu);//遍历转发共享通知
            qDebug()<<caRecvName;
        }
        free(respdu);
        respdu=NULL;

        respdu=mkPDU(0);//给分享者的回复
        respdu->uiMsgType=ENUM_MSG_SHARE_FILE_RESPAND;
        strcpy(respdu->caData,"share file ok~");
        this->write((char*)respdu,respdu->uiPDULen);

        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_SHARE_FILE_NOTICE_RESPOND://分享文件通知的客户端回复
    {
        QString strRecvPath=QString("../user/%1/share").arg(pdu->caData);
        QString strShareFilePath=QString((char*)pdu->caMsg);

        int index=strShareFilePath.lastIndexOf('/');
        QString strFileName=strShareFilePath.right(strShareFilePath.size()-index-1);

        QFileInfo fileInfo(strShareFilePath);
        if(fileInfo.isFile())
        {
            QFile::copy(strShareFilePath,strRecvPath+"/"+strFileName);
        }
        else if(fileInfo.isDir())
        {
            this->copyDir(strShareFilePath,strRecvPath+"/"+strFileName);
        }
        break;
    }
    case ENUM_MSG_MOVE_FILE_REQUEST://移动（剪切和粘贴）文件请求
    {
        int srcLen=0;//剪切路径长度
        int destLen=0;//粘贴路径长度
        sscanf(pdu->caData,"%d %d",&srcLen,&destLen);
        char *pSrcPath=new char[srcLen+1];
        char *pDestPath=new char[destLen+1];
        memset(pSrcPath,'\0',srcLen+1);//清空
        memset(pDestPath,'\0',destLen+1);
        memcpy(pSrcPath,(char*)pdu->caMsg,srcLen);
        memcpy(pDestPath,(char*)pdu->caMsg+(srcLen+1),destLen);

        qDebug()<<pSrcPath;
        qDebug()<<pDestPath;

        bool res=false;
        QFileInfo fileInfo(pSrcPath);
        if(fileInfo.isFile())
        {
            res=QFile::rename(pSrcPath,pDestPath);
        }
        else
        {
            QDir dir;
            res = dir.rename(pSrcPath,pDestPath);
        }
        PDU *respdu=mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_MOVE_FILE_RESPAND;//移动文件回复pdu
        if(res)
        {
            strcpy(respdu->caData,FILE_MOVE_OK);
        }
        else
        {
            strcpy(respdu->caData,FILE_MOVE_FAILED);
        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_LOGOFF_REQUEST:
    {
        bool res=OperateDB::getInstance().handleLogoff(caName,caPwd);
        PDU *respdu=mkPDU(0);
        respdu->uiMsgType=ENUM_MSG_LOGOFF_RESPAND;//注销账号回复pdu
        if(res)
        {
            strcpy(respdu->caData,"Logout successful");
        }
        else
        {
            strcpy(respdu->caData,"Unregistration failed");
        }
        this->write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    default:
    {
        break;
    }
    }
    free(pdu);
    pdu=NULL;
}

void MyTcpSocket::recvDataStream()//收发二进制数据流
{
    qDebug()<<m_File.fileName();

    // QByteArray buffer;
    // qint64 res=0;
    if(this->m_File.open(QIODevice::WriteOnly))
    {
        // while (true)
        // {
        //     res=this->read(buffer.data(),4096);

        //     this->m_File.write(buffer);//全部写入
        //     this->m_iRecved+=buffer.size();

        //     buffer.clear();

        //     if (res==0)
        //         break;
        // }
        QByteArray buffer=this->readAll();//一直读取数据，直到设备没有更多的数据可供读取，或者发生了错误。
        qDebug()<<buffer.data();//传过来的数据
        this->m_File.write(buffer);//全部写入
        this->m_iRecved+=buffer.size();

        m_File.close();//关闭文件
    }
    PDU *respdu=mkPDU(this->m_File.fileName().size()+128);
    respdu->uiMsgType=ENUM_MSG_UPLOAD_FILE_RESPAND;
    this->m_bIsDataStream=false;

    if(this->m_iRecved == this->m_iTotal)//上传成功
    {
        strcpy((char*)respdu->caMsg,(this->m_File.fileName()+" "+FILE_UPLOAD_OK).toUtf8().cbegin());

    }
    else//上传失败
    {
        strcpy((char*)respdu->caMsg,(this->m_File.fileName()+" "+FILE_UPLOAD_FAILED).toUtf8().cbegin());
        this->m_File.remove();//删除创建的空文件
    }
    this->write((char*)respdu,respdu->uiPDULen);
    free(respdu);
    respdu=NULL;
}

void MyTcpSocket::clientOffline()//处理用户下线
{
    //设置在线状态
    OperateDB::getInstance().handleOffline(m_strName.toStdString().c_str()); //参数为char*
    emit offline(this);//this 该对象的地址
}

void MyTcpSocket::sendFileToClient()//发送给客户端下载文件的内容
{
    this->m_pTimer->stop();

    //打开文件
    qDebug()<<"打开文件"<<this->m_File.fileName();
    if(this->m_File.open(QIODevice::ReadOnly))
    {
        char *pData=new char[4096];
        qint64 res=0;
        while(true)
        {
            res=this->m_File.read(pData,4096);//读文件操作
            if(res>0&&res<=4096)
            {
                this->write(pData,res);
                // qDebug()<<QString(pData);
            }
            else
            {
                this->m_File.close();
                break;
            }
        }
        delete []pData;
        pData=NULL;
    }
}
