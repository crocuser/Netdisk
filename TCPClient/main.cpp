#include "tcpclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //完成一个功能测试一下owo哦~
    // AllOnline w;
    // w.show();


    //只有一个对象
    TCPClient::getInstance().show();

    return a.exec();
}
