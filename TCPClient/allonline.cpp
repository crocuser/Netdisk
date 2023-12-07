#include "allonline.h"
#include "ui_allonline.h"
#include "tcpclient.h"
#include <QMessageBox>

AllOnline::AllOnline(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AllOnline)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
}

AllOnline::~AllOnline()
{
    delete ui;
}

void AllOnline::showUser(PDU *pdu)
{
    if(NULL==pdu) return;
    uint uiSize=pdu->uiMsgLen/32;
    char caTmp[32];
    ui->all_online_list->clear(); // 清空列表

    // qDebug()<<"你好";
    // if(pdu->uiMsgType==ENUM_MSG_ALL_ONLINE_RESPOND)
    // {
    ui->all_online_list->addItem("所有在线用户如下：");
    ui->all_online_list->addItem("666");
    for(uint i=0;i<uiSize;++i)
    {
        memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32);
        ui->all_online_list->addItem(caTmp);
    }
    // }

}

void AllOnline::on_add_friend_pb_clicked()
{
    //获取左边选择的用户
    QListWidgetItem *pItem=ui->all_online_list->currentItem();
    QString strFriendName=pItem->text();//未来好友的名字
    if(0==strcmp(strFriendName.toStdString().c_str(),"所有在线用户如下："))
    {
        QMessageBox::information(this,"添加好友","请选择想要添加的好友！");
        return;
    }
    QString strLoginName=TCPClient::getInstance().getLonginName();//我的名字

    PDU *pdu=mkPDU(0);
    pdu->uiMsgType=ENUM_MSG_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData,strFriendName.toStdString().c_str(),strFriendName.size());//朋友
    memcpy(pdu->caData+32,strLoginName.toStdString().c_str(),strLoginName.size());//我自己
    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

