#include "tcpclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //功能测试
    // AllOnline w;
    // w.show();


    //只有一个对象
    TCPClient::getInstance().show();

    return a.exec();
}
