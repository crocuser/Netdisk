#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QMessageBox>
#include <QDateTime>

PrivateChat::PrivateChat(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle(TCPClient::getInstance().getLonginName()+" 收到的私聊");
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

PrivateChat &PrivateChat::getInstance()
{
    static PrivateChat instance;
    return instance;
}

void PrivateChat::setChatName(QString strName)
{
    this->m_strChatName=strName;//传过来的名字保存在成员变量中
    this->m_strLoginName=TCPClient::getInstance().getLonginName();//自己的名字
}

void PrivateChat::updateMessageList(const PDU *pdu)//更新消息列表
{
    if(NULL==pdu) return;
    this->update();//刷新窗口
    this->show();//显示私聊窗口

    char caSendName[32]={'\0'};//发送方，即我
    memcpy(caSendName,pdu->caData+32,32);
    this->setChatName(caSendName);//设置发送信息的用户
    QString strMessage=QString("%1 : %2").arg(caSendName).arg((char*)(pdu->caMsg));
    ui->showMessage_te->append(strMessage);

}

void PrivateChat::on_sendMessage_pb_clicked()
{
    //发送消息
    QString strMessage=ui->inputMessage_le->text();
    ui->inputMessage_le->clear();//发完消息后，清空输入框
    if(strMessage.isEmpty())
    {
        QMessageBox::warning(this,"私聊","发送的消息不能为空！");
        return;
    }
    QDateTime currentDateTime = QDateTime::currentDateTime();
    strMessage=strMessage+"--"+currentDateTime.toString("yyyy-MM-dd hh:mm:ss");//添加发送时间
    //防中文乱码
    QByteArray byteArray = strMessage.toUtf8();

    PDU *pdu=mkPDU(byteArray.size()+1);
    pdu->uiMsgType=ENUM_MSG_PRIVATE_CHAT_REQUEST;
    memcpy(pdu->caData,m_strChatName.toStdString().c_str(),m_strChatName.size());
    memcpy(pdu->caData+32,m_strLoginName.toStdString().c_str(),m_strLoginName.size());//自己的名字放在后面

    strcpy((char*)(pdu->caMsg),byteArray);//消息内容

    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

