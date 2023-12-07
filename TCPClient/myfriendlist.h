#ifndef MYFRIENDLIST_H
#define MYFRIENDLIST_H

#include <QWidget>

#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "allonline.h"

class MyFriendList : public QWidget
{
    Q_OBJECT
public:
    explicit MyFriendList(QWidget *parent = nullptr);
    void showAllOnlineUser(PDU *pdu);
    void showUpdateFriendList(PDU *pdu);
    void showUpateMessageList(PDU *pdu);

    QListWidget *getAllFriendList();//获得在线好友列表

    QString m_strSearchName;//临时存储搜索用户名

signals:

public slots:
    void showOnline();
    void searchUser();
    void flushFriend();
    void deleteFriend();
    void privateChat();
    void groupChat();

private:
    QTextEdit *m_pShowMsgTE;//显示信息
    QListWidget *m_pFriendListWidget;//显示好友列表
    QLineEdit *m_pInputMsgLE;//信息输入框

    QPushButton *m_pDeleteFriendPB;//删除好友按钮
    QPushButton *m_pFlushFriendPB;//刷新好友列表
    QPushButton *m_pShowOnlineUserPB;//查看所有在线用户
    QPushButton *m_pSearchUserPB;//搜索用户按钮

    QPushButton *m_pMsgSendPB;//发送消息的按钮
    QPushButton *m_pPrivateChatPB;//私聊按钮，默认是群聊

    AllOnline *m_pAllOnline;


};

#endif // MYFRIENDLIST_H
