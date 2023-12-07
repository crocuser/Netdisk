#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"

#include <QByteArray> //字节数组
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <QFile>

TCPServer::TCPServer(QWidget *parent):
    QWidget(parent),
    ui(new Ui::TCPServer)
{
    ui->setupUi(this);
    loadConfig(); //调用
    MyTCPServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);//监听
}

TCPServer::~TCPServer()
{
    delete ui;
}

void TCPServer::loadConfig()
{
    QFile file(":/server.config");
    if (file.open(QIODevice::ReadOnly)) //只读打开
    {
        QByteArray baData=file.readAll();//全部读出，返回一个字节数组
        QString strData=baData.toStdString().c_str();//字节转为字符串，给出首地址
        // qDebug() << strData; //打印
        file.close();

        //对数据进行处理
        strData.replace("\r\n"," "); //替换

        QStringList strList=strData.split(" ");//空格分割

        m_strIP=strList.at(0);
        m_usPort=strList.at(1).toUShort();//转成整型
        qDebug()<<"ip:"<<m_strIP<<"\tport:"<<m_usPort;
    }
    else
    {
        QMessageBox::critical(this,"open config","open config failed");
    }
}
