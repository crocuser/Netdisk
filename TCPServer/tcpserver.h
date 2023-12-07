#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class TCPServer;
}
QT_END_NAMESPACE

class TCPServer : public QWidget
{
    Q_OBJECT

public:
    TCPServer(QWidget *parent = nullptr);
    ~TCPServer();
    void loadConfig();

private:
    Ui::TCPServer *ui;
    QString m_strIP; //存放ip
    quint16 m_usPort; //(无符号16位整数)存放端口号
};
#endif // TCPSERVER_H
