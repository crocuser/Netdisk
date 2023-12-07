#ifndef OPERATEWIDGET_H
#define OPERATEWIDGET_H

#include <QWidget>
#include <QListWidget>

#include "myfriendlist.h"
#include "netdiskfile.h"
#include <QStackedWidget> //栈窗口，每次只显示一个窗口

class OperateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OperateWidget(QWidget *parent = nullptr);
    static OperateWidget &getInstance();
    MyFriendList *getFriendList();
    NetdiskFile *getNetdisFile();

signals:

private:
    QListWidget *m_pListW;
    MyFriendList *m_pMyFriendList;
    NetdiskFile *m_pNetdiskFile;

    QStackedWidget *m_pStackedW;
};

#endif // OPERATEWIDGET_H
