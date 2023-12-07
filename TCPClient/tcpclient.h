#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile> //使用这个类将文件读取出来
#include <QTcpSocket>

#include <QRegularExpression> //正则匹配

#include "operatewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TCPClient;
}
QT_END_NAMESPACE

class TCPClient : public QWidget
{
    Q_OBJECT

public:
    TCPClient(QWidget *parent = nullptr);
    ~TCPClient();
    //成员函数：加载配置
    void loadConfig();//声明

    //单例模式
    static TCPClient &getInstance();
    //使用tcpsocket发送请求，同一个对象，其他类也可以使用
    QTcpSocket &getTcpSocket();//这个是私有属性

    //公共接口：获取当前登录用户名
    QString getLonginName();
    QString getCurrentPath();
    void setCurrentPath(QString strPath);

    //实现哈希加密算法
    QString hashString(const QString &inputString);

//槽函数
public slots:
    void showConnect();//信号的处理函数，用来检查用户是否成功连接服务器
    void recvMsg();//接收服务器发来的数据

private slots:
    // void on_send_pb_clicked();//之前用来测试的，没用了

    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_logoff_pb_clicked();

private:
    Ui::TCPClient *ui;
    QString m_strIP; //存放ip
    quint16 m_usPort; //(无符号16位整数)存放端口号

    //连接服务器，和服务器数据交互
    QTcpSocket m_tcpSocket;

    //记录当前登录用户
    QString m_strLoginName;

    //记录当前所处路径
    QString m_strCurrentPath;

};
#endif // TCPCLIENT_H
