#include "mytcpserver.h"
#include <QDebug>


MyTCPServer::MyTCPServer()
{

}

MyTCPServer &MyTCPServer::getInstance()
{
    static MyTCPServer instance;
    return instance;
}

void MyTCPServer::incomingConnection(qintptr socketDescriptor)//传来一个socket描述符
{
    //一旦有客户端连接过来，就会调用这个函数
    qDebug()<<"new client connected";
    MyTcpSocket *pTcpSocket=new MyTcpSocket;

    pTcpSocket->open(QIODevice::ReadWrite);
    pTcpSocket->setTextModeEnabled(true);

    pTcpSocket->setSocketDescriptor(socketDescriptor);//设置socket描述符
    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),this,SLOT(deleteSocket(MyTcpSocket*)));
}

void MyTCPServer::resend(const char *friendName, PDU *pdu)
{
    if(friendName==NULL || pdu==NULL)   return;
    //遍历链表
    QString strName=friendName;
    for(int i=0;i<m_tcpSocketList.size();++i)
    {
        if(strName==m_tcpSocketList.at(i)->getName())
        {
            m_tcpSocketList.at(i)->write((char*)pdu,pdu->uiPDULen);
            break;
        }
    }

}

void MyTCPServer::deleteSocket(MyTcpSocket *mySocket)
{
    //遍历list，然后删除
    QList<MyTcpSocket*>::iterator iter=m_tcpSocketList.begin();
    for(;iter!=m_tcpSocketList.end();++iter)
    {
        if(mySocket==*iter)
        {
            delete *iter;//new的对象删除
            *iter=NULL;
            m_tcpSocketList.erase(iter);//指针删除
            break;
        }
    }
    for(int i=0;i<m_tcpSocketList.size();++i)
    {
        qDebug()<<m_tcpSocketList.at(i)->getName();
    }
}
