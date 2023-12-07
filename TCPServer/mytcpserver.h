#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"


class MyTCPServer : public QTcpServer
{
    //想让自己的类支持信号槽
    Q_OBJECT
public:
    MyTCPServer();
    static MyTCPServer& getInstance();

    //重写父类的虚函数
    void incomingConnection(qintptr socketDescriptor);

    //转发数据
    void resend(const char *friendName,PDU *pdu);

public slots:
    void deleteSocket(MyTcpSocket *mySocket);//槽函数和信号的函数一致

private:
    //存储socket
    QList<MyTcpSocket*> m_tcpSocketList;

};

#endif // MYTCPSERVER_H
