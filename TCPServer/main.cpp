#include "tcpserver.h"

#include <QApplication>
#include "operatedb.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //测试数据库连接
    OperateDB::getInstance().init();

    TCPServer w;
    w.show();
    return a.exec();
}
