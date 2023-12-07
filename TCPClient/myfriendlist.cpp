#include "myfriendlist.h"
#include "protocol.h" //协议：封装信息
#include "tcpclient.h" //发送请求
#include <QInputDialog> //输入查询的用户
#include <QDebug>
#include <QMessageBox>
#include "privatechat.h"
#include <QDateTime>

MyFriendList::MyFriendList(QWidget *parent)
    : QWidget{parent}
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    m_pShowMsgTE = new QTextEdit;//显示信息
    m_pShowMsgTE->setDocumentTitle("群发消息如下：");
    // m_pShowMsgTE->setFixedWidth(450);
    m_pFriendListWidget = new QListWidget;//显示好友列表
    m_pFriendListWidget->setFixedWidth(150);
    m_pInputMsgLE = new QLineEdit;//信息输入框
    // m_pInputMsgLE->setFixedWidth(350);

    m_pDeleteFriendPB = new QPushButton("删除好友");//删除好友按钮
    m_pFlushFriendPB = new QPushButton("刷新好友");//刷新好友列表
    m_pShowOnlineUserPB = new QPushButton("显示在线用户");//查看所有在线用户
    m_pSearchUserPB = new QPushButton("查找用户");//搜索用户按钮

    m_pMsgSendPB = new QPushButton("发送");//发送消息的按钮
    m_pMsgSendPB->setFixedWidth(100);
    m_pPrivateChatPB = new QPushButton("私聊");//私聊按钮，默认是群聊

    m_pAllOnline = new AllOnline;//所有在线用户列表

    //布局
    //按钮垂直布局
    QVBoxLayout *pRightPBVBL = new QVBoxLayout;
    pRightPBVBL->addWidget(m_pDeleteFriendPB);
    pRightPBVBL->addWidget(m_pFlushFriendPB);
    pRightPBVBL->addWidget(m_pShowOnlineUserPB);
    pRightPBVBL->addWidget(m_pSearchUserPB);
    pRightPBVBL->addWidget(m_pPrivateChatPB);


    //水平布局：消息输入框，发送按钮
    QHBoxLayout *pMsgHBL = new QHBoxLayout;
    pMsgHBL->addWidget(m_pInputMsgLE);
    pMsgHBL->addWidget(m_pMsgSendPB);

    //垂直布局消息展示框和输入框、发送按钮
    QVBoxLayout *pLeftVBL = new QVBoxLayout;
    pLeftVBL->addWidget(m_pShowMsgTE);
    pLeftVBL->addLayout(pMsgHBL);

    //水平布局:消息展示框，好友列表，按钮垂直布局
    QHBoxLayout *pRightHBL = new QHBoxLayout;
    pRightHBL->addLayout(pLeftVBL);
    pRightHBL->addWidget(m_pFriendListWidget);
    pRightHBL->addLayout(pRightPBVBL);

    //整体垂直布局
    QVBoxLayout *pMain = new QVBoxLayout;
    pMain->addLayout(pRightHBL);

    // pMain->addWidget(m_pAllOnline);//不显示是因为online最后面没有选择主窗口为水平布局
    //但是我想弹出新窗口

    m_pAllOnline->hide();

    setLayout(pMain);

    //点击按钮则触发槽函数
    connect(m_pShowOnlineUserPB,SIGNAL(clicked(bool)),this,SLOT(showOnline()));//所有在线用户
    connect(m_pSearchUserPB,SIGNAL(clicked(bool)),this,SLOT(searchUser()));//搜索用户
    connect(m_pFlushFriendPB,SIGNAL(clicked(bool)),this,SLOT(flushFriend()));//刷新好友
    connect(m_pDeleteFriendPB,SIGNAL(clicked(bool)),this,SLOT(deleteFriend()));//删除好友
    connect(m_pPrivateChatPB,SIGNAL(clicked(bool)),this,SLOT(privateChat()));//私聊好友
    connect(m_pMsgSendPB,SIGNAL(clicked(bool)),this,SLOT(groupChat()));//群聊好友
}

void MyFriendList::showAllOnlineUser(PDU *pdu)
{
    if(NULL==pdu) return;
    m_pAllOnline->showUser(pdu);
}

void MyFriendList::showUpateMessageList(PDU *pdu)//显示更新后的群聊界面
{
    m_pShowMsgTE->append((char*)(pdu->caMsg));
    m_pShowMsgTE->update();
}

QListWidget *MyFriendList::getAllFriendList()
{
    return this->m_pFriendListWidget;
}

void MyFriendList::showUpdateFriendList(PDU *pdu)//显示更新后的好友列表
{
    if(NULL==pdu) return;
    uint uiSize=pdu->uiMsgLen/32;
    char caTmp[32]={'\0'};
    m_pFriendListWidget->clear();
    m_pFriendListWidget->addItem("我的好友列表如下；");
    for(uint i=0;i<uiSize;++i)
    {
        memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32);
        QString str=caTmp;
        QString user=str.trimmed();//去空格
        QStringList strList = user.split(",");//逗号分割
        if(strList.size()!=2) continue;
        if(strList.at(1)=="0")
        {
            m_pFriendListWidget->addItem(strList.at(0)+"\t 已下线");
        }
        else if(strList.at(1)=="1")
        {
            m_pFriendListWidget->addItem(strList.at(0)+"\t 在线");
        }
    }

}

void MyFriendList::showOnline()
{
    if(m_pAllOnline->isHidden())
    {
        m_pAllOnline->show();

        //请求：显示所有在线用户
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_ALL_ONLINE_REQUEST;
        TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送pdu
        free(pdu);
        pdu=NULL;
    }
    else
    {
        m_pAllOnline->hide();
    }
}

void MyFriendList::searchUser()
{
    m_strSearchName=QInputDialog::getText(this,"搜索","用户名");
    if(!m_strSearchName.isEmpty())
    {
        qDebug()<<m_strSearchName;
        PDU *pdu=mkPDU(0);
        pdu->uiMsgType=ENUM_MSG_SEARCH_USERS_REQUEST;
        memcpy((char*)(pdu->caData),m_strSearchName.toStdString().c_str(),m_strSearchName.size());//被赋值地址，值，大小
        TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送pdu
        free(pdu);
        pdu=NULL;
    }
}

void MyFriendList::flushFriend()
{
    QString strName=TCPClient::getInstance().getLonginName();
    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());
    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void MyFriendList::deleteFriend()
{
    //指针先判空
    if(NULL==m_pFriendListWidget->currentItem()||m_pFriendListWidget->currentItem()->text()=="我的好友列表如下；")
    {
        QMessageBox::information(this,"删除好友","请选择想要删除的好友");
        return;
    }
    QString strFriend=m_pFriendListWidget->currentItem()->text();//当前用户选中的好友条目
    qDebug()<<strFriend;
    QStringList strList = strFriend.split("\t");//朋友条目需要切割

    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_DELETE_FRIEND_REQUEST;
    QString strMyName=TCPClient::getInstance().getLonginName();
    memcpy(pdu->caData,strList[0].toStdString().c_str(),strList[0].size());
    memcpy(pdu->caData+32,strMyName.toStdString().c_str(),strMyName.size());

    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void MyFriendList::privateChat()//私聊
{
    //指针先判空
    if(NULL==m_pFriendListWidget->currentItem()||m_pFriendListWidget->currentItem()->text()=="我的好友列表如下；")
    {
        QMessageBox::information(this,"私聊好友","请选择想要私聊的对象");
        return;
    }
    QStringList strList=m_pFriendListWidget->currentItem()->text().split("\t");
    QString strChatName=strList.at(0);
    PrivateChat::getInstance().setChatName(strChatName);//设置私聊对象和我的名字
    if(PrivateChat::getInstance().isHidden())//显示私聊窗口
    {
        PrivateChat::getInstance().show();
    }
}

void MyFriendList::groupChat()//群聊
{  
    if(m_pInputMsgLE->text().isEmpty())
    {
        QMessageBox::warning(this,"群聊","信息不能为空");
        return;
    }
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString strMessage=QString("%1 : %2--%3").arg(TCPClient::getInstance().getLonginName()).arg(m_pInputMsgLE->text()).arg(currentDateTime.toString("yyyy-MM-dd hh:mm:ss"));
    m_pInputMsgLE->clear();//发完清空
    QByteArray byteArray = strMessage.toUtf8();//防中文乱码

    PDU *pdu=mkPDU(byteArray.size()+1);
    pdu->uiMsgType=ENUM_MSG_GROUP_CHAT_REQUEST;
    QString name=TCPClient::getInstance().getLonginName();
    memcpy(pdu->caData,name.toStdString().c_str(),name.size());


    memcpy((char*)pdu->caMsg,byteArray,byteArray.size());
    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}
