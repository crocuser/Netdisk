#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include <QFile>
#include <QTimer>


class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit MyTcpSocket(QObject *parent = nullptr);
    QString getName();

    void copyDir(QString strSrcDir,QString strDestDir);//拷贝目录


//定义信号
signals:
    void offline(MyTcpSocket *mysocket);


//槽函数
public slots:
    void recvMsg();//用于收发协议数据
    void recvDataStream();//用于收发二进制数据流
    void clientOffline();
    void sendFileToClient();//发送文件给客户端

//保存登录的用户名
private:
    QString m_strName;//在客户端断开服务器后，向服务器发送disconnect()信号，以让服务器修改在线状态

    QFile m_File;//传输文件
    qint64 m_iTotal;//文件大小
    qint64 m_iRecved;//收到的数据大小
    bool m_bIsDataStream=false;//默认传输协议数据
    QTimer *m_pTimer=new QTimer;
};

#endif // MYTCPSOCKET_H
