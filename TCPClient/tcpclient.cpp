#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray> //字节数组
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>

#include "protocol.h" //协议数据单元
#include "privatechat.h"
#include <QCryptographicHash>//使用 QCryptographicHash::Sha256 来创建一个SHA-256哈希对象

TCPClient::TCPClient(QWidget *parent): QWidget(parent) ,ui(new Ui::TCPClient)
{
    ui->setupUi(this);
    // resize(500,250);//调整窗口大小
    this->setAttribute(Qt::WA_DeleteOnClose);//解决叉掉就崩溃的问题

    loadConfig();
    //关联信号和信号处理函数：信号的发送方，信号，信号的接收方，处理函数
    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));//一旦服务器连接成功，则给用户反馈信息

    connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));//一旦服务器发送来数据，则接收，并处理

    //连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort); //默认读写
}

TCPClient::~TCPClient()
{
    delete ui;
}

void TCPClient::loadConfig()//定义
{
    QFile file(":/client.config");
    if (file.open(QIODevice::ReadOnly)) //只读打开
    {
        QByteArray baData=file.readAll();//全部读出，返回一个字节数组
        QString strData=baData.toStdString().c_str();//字节转为字符串，给出首地址
        // qDebug() << strData; //打印
        file.close();

        //对数据进行处理
        strData.replace("\r\n"," "); //替换

        QStringList strList=strData.split(" ");//空格分割

        m_strIP=strList.at(0);
        m_usPort=strList.at(1).toUShort();//转成整型

    }
    else
    {
        QMessageBox::critical(this,"open config","open config failed");
    }
}

TCPClient &TCPClient::getInstance()
{
    static TCPClient instance;
    return instance;
}

QTcpSocket &TCPClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TCPClient::getLonginName()
{
    return m_strLoginName;
}

QString TCPClient::getCurrentPath()
{
    return m_strCurrentPath;
}

void TCPClient::setCurrentPath(QString strPath)
{
    this->m_strCurrentPath=strPath;
}

//实现哈希加密算法
QString TCPClient::hashString(const QString &inputString)
{
    // 创建 QCryptographicHash 对象，选择 SHA-256 算法
    QCryptographicHash hash(QCryptographicHash::Sha256);

    // 更新哈希对象的输入
    hash.addData(inputString.toUtf8());

    // 获取十六进制表示的哈希值--64位
    QByteArray hashedData = hash.result();
    QString hashedString = hashedData.toHex().right(32);//获取后32位

    return hashedString;
}

void TCPClient::showConnect()
{
    //给用户反馈消息，会弹出一个消息提示框
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}

void TCPClient::recvMsg()
{
    if(OperateWidget::getInstance().getNetdisFile()->getIsDownload())
    {
        OperateWidget::getInstance().getNetdisFile()->recvFileData();
        return;
    }

    // qDebug()<<m_tcpSocket.bytesAvailable();
    uint uiPDULen=0;
    m_tcpSocket.read((char*)&uiPDULen,sizeof(uint));
    //实际消息长度
    uint uiMsgLen=uiPDULen-sizeof(PDU);
    PDU *pdu=mkPDU(uiMsgLen); //制造pdu
    m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));//读数据

    //不同的消息类型，对应不同的处理方式
    switch(pdu->uiMsgType)
    {
    case ENUM_MSG_REGIST_RESPOND://注册回复
    {
        if(0==strcmp(pdu->caData,REGIST_OK))
        {
            QMessageBox::information(this,"注册",REGIST_OK);
        }
        else
        {
            QMessageBox::information(this,"注册",REGIST_FAILED);
        }
        break;
    }
    case ENUM_MSG_LOGIN_RESPOND://登录回复
    {
        if(0==strcmp(pdu->caData,LOGIN_OK))
        {
            m_strCurrentPath=QString("../user/%1").arg(m_strLoginName);//记录当前目录为根目录
            QMessageBox::information(this,"登录",LOGIN_OK);
            OperateWidget::getInstance().show();
            this->hide();//隐藏登录注册注销界面
        }
        else
        {
            QMessageBox::information(this,"登录",LOGIN_FAILED);
        }
        break;
    }
    case ENUM_MSG_ALL_ONLINE_RESPOND://所有在线用户回复
    {
        OperateWidget::getInstance().getFriendList()->showAllOnlineUser(pdu);
        break;
    }
    case ENUM_MSG_SEARCH_USERS_RESPOND://搜索用户回复
    {
        QString name=OperateWidget::getInstance().getFriendList()->m_strSearchName;
        uint uiSize=(pdu->uiMsgLen)/32;//这是搜索到的人数

        if(uiSize==0)
        {
            QMessageBox::information(this,"搜索",QString("%1 相关用户不存在！").arg(name));
        }
        else
        {
            // OperateWidget::getInstance().getFriendList()->showAllOnlineUser(pdu);//让所有在线用户那个界面去显示，这次会多了用户是否在线
            QWidget *qw=new QWidget;
            qw->setAttribute(Qt::WA_DeleteOnClose);
            QListWidget *w=new QListWidget(nullptr);
            char caTmp[32];
            w->addItem("搜索结果如下；");
            for(uint i=0;i<uiSize;++i)
            {
                memcpy(caTmp,(char*)(pdu->caMsg)+i*32,32);
                QString str=caTmp;
                QString user=str.trimmed();//去空格
                QStringList strList = user.split(",");//逗号分割
                if(strList.size()!=2) continue;
                if(strList.at(1)=="0")
                {
                    w->addItem(strList.at(0)+"\t 已下线");
                }
                else if(strList.at(1)=="1")
                {
                    w->addItem(strList.at(0)+"\t 在线");
                }

            }
            QHBoxLayout *pMain = new QHBoxLayout;
            pMain->addWidget(w);
            qw->setLayout(pMain);
            qw->show();
        }
        break;
    }
    case ENUM_MSG_ADD_FRIEND_RESPOND://添加好友的回复
    {
        QMessageBox::information(this,"添加好友",pdu->caData);
        break;
    }
    case ENUM_MSG_ADD_FRIEND_REQUEST://添加好友的请求
    {
        char friendName[32]={'\0'};
        strncpy(friendName,pdu->caData+32,32);//提出请求的人，即我
        //给qmessageBox添加按钮，获得用户的选择，是同意还是拒绝
        int res=QMessageBox::information(this,"添加好友",QString("%1 want to add you as friend?").arg(friendName),QMessageBox::Yes,QMessageBox::No);
        PDU *respdu=mkPDU(0);
        memcpy(respdu->caData,pdu->caData,64);
        if(res==QMessageBox::Yes)
        {
            respdu->uiMsgType=ENUM_MSG_ADD_FRIEND_AGREE;
        }
        else //if(res==QMessageBox::No)
        {
            respdu->uiMsgType=ENUM_MSG_ADD_FRIEND_REFUSE;
        }
        m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
        free(respdu);
        respdu=NULL;
        break;
    }
    case ENUM_MSG_ADD_FRIEND_AGREE://回复我：同意
    {
        char friendName[32]={'\0'};
        strncpy(friendName,pdu->caData,32);
        QMessageBox::information(this,"添加好友",QString("%1 同意了你的好友请求").arg(friendName));
        break;
    }
    case ENUM_MSG_ADD_FRIEND_REFUSE://回复我：拒绝
    {
        char friendName[32]={'\0'};
        strncpy(friendName,pdu->caData,32);
        QMessageBox::information(this,"添加好友",QString("%1 拒绝了你的好友请求").arg(friendName));
        break;
    }
    case ENUM_MSG_FLUSH_FRIEND_RESPOND://刷新好友回复
    {
        OperateWidget::getInstance().getFriendList()->showUpdateFriendList(pdu);
        break;
    }
    case ENUM_MSG_DELETE_FRIEND_RESPOND://删除好友回复
    {
        QMessageBox::information(this,"删除好友",pdu->caData);
        break;
    }
    case ENUM_MSG_PRIVATE_CHAT_REQUEST://私聊请求
    {
        PrivateChat::getInstance().updateMessageList(pdu);
        break;
    }
    case ENUM_MSG_GROUP_CHAT_REQUEST://群聊请求
    {
        OperateWidget::getInstance().getFriendList()->showUpateMessageList(pdu);
        break;
    }
    case ENUM_MSG_CREATE_DIR_RESPOND://新建目录回复
    {
        QMessageBox::information(this,"新建目录",QString(pdu->caData));
        break;
    }
    case ENUM_MSG_FLUSH_DIR_RESPOND://刷新目录回复
    {
        OperateWidget::getInstance().getNetdisFile()->showDirContent(pdu);
        break;
    }
    case ENUM_MSG_DELETE_FILE_RESPOND://删除文件回复
    {
        QMessageBox::information(this,"删除文件",QString((char*)pdu->caMsg));
        break;
    }
    case ENUM_MSG_RENAME_FILE_RESPAND://重命名文件回复
    {
        QMessageBox::information(this,"重命名文件",QString((char*)pdu->caMsg));
        break;
    }
    case ENUM_MSG_CREATE_FILE_RESPAND://新建文件回复
    {
        QMessageBox::information(this,"新建文件",QString(pdu->caData));
        break;
    }
    case ENUM_MSG_UPLOAD_FILE_RESPAND://上传文件回复
    {
        QMessageBox::information(this,"上传文件",QString((char*)pdu->caMsg));
        break;
    }
    case ENUM_MSG_DOWNLOAD_FILE_RESPAND://下载文件回复
    {
        qDebug()<<pdu->caData;
        char caFileName[32]={'\0'};
        sscanf(pdu->caData,"%s %11lld",caFileName,&(OperateWidget::getInstance().getNetdisFile()->m_iDownloadFileSize));//文件名，文件大小
        if(strlen(caFileName)>0 && OperateWidget::getInstance().getNetdisFile()->m_iDownloadFileSize>0)//判断数据有效性
        {
            OperateWidget::getInstance().getNetdisFile()->setIsDownload(true);//可以接收数据流了
            qDebug()<<caFileName<<":"<<OperateWidget::getInstance().getNetdisFile()->m_iDownloadFileSize;
        }

        break;
    }
    case ENUM_MSG_SHARE_FILE_RESPAND://分享文件回复
    {
        QMessageBox::information(this,"分享文件",pdu->caData);
        break;
    }
    case ENUM_MSG_SHARE_FILE_NOTICE://分享文件通知
    {
        char *pPath=new char[pdu->uiMsgLen];
        memcpy(pPath,pdu->caMsg,pdu->uiMsgLen);
        qDebug()<<pPath;
        char *pos=strrchr(pPath,'/');//用于在字符串中查找指定字符的最后一次出现位置
        if(NULL!=pos)
        {
            pos++;
            QString strNotice=QString("%1 share file: %2. \nDo you accept it?").arg(pdu->caData).arg(pos);
            int res=QMessageBox::question(this,"分享文件",strNotice);//提示信息
            if(QMessageBox::Yes==res)
            {
                PDU *respdu=mkPDU(pdu->uiMsgLen);
                respdu->uiMsgType=ENUM_MSG_SHARE_FILE_NOTICE_RESPOND;//分享文件通知的回复

                QString strName=TCPClient::getInstance().getLonginName();//接收方的名字
                qDebug()<<strName;
                strcpy(respdu->caData,strName.toStdString().c_str());
                memcpy((char*)respdu->caMsg,(char*)pdu->caMsg,pdu->uiMsgLen);

                this->m_tcpSocket.write((char*)respdu,respdu->uiPDULen);//给服务器反馈
                free(respdu);
                respdu=NULL;
            }
        }
        break;
    }
    case ENUM_MSG_MOVE_FILE_RESPAND://粘贴文件回复
    {
        QMessageBox::information(this,"粘贴文件",pdu->caData);
        break;
    }
    case ENUM_MSG_LOGOFF_RESPAND://注销账号回复
    {
        QMessageBox::information(this,"注销账号",pdu->caData);
        break;
    }
    default:
    {
        break;
    }
    }
    free(pdu);
    pdu=NULL;
}

#if 0
//只要我们点击发送就会跳到这里来，然后判断，再发送给服务器
void TCPClient::on_send_pb_clicked()
{
    QString strMsg=ui->lineEdit->text();
    if(!strMsg.isEmpty())
    {
        PDU *pdu=mkPDU(strMsg.size());
        pdu->uiMsgType=8888;//消息类型，测试用
        QString data="filenme";
        memcpy(pdu->caData,data.toStdString().c_str(),data.size());
        memcpy(pdu->caMsg,strMsg.toStdString().c_str(),strMsg.size());//把要发送的数据拷贝到数据单元中
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        // qDebug()<<"这是什么情况";
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::warning(this,"信息发送","发送信息不能为空");
    }
}
#endif

//使用槽处理用户提交的信息
//登录按钮关联事件
void TCPClient::on_login_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();

    if(!strName.isEmpty()&&!strPwd.isEmpty())
    {
        m_strLoginName=strName;//记录当前登录用户名
        PDU *pdu=mkPDU(0);//没有msg
        pdu->uiMsgType=ENUM_MSG_LOGIN_REQUEST;
        // 都放到caData
        strPwd=hashString(strPwd);//加密
        // qDebug()<<strPwd;

        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        //发送给服务器
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
    else
    {
        QMessageBox::critical(this,"登录","登录失败：用户名或密码不能为空！");
    }
}

//注册按钮关联事件
void TCPClient::on_regist_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();

    //对用户名和密码进行一些简单的正则匹配
    static QRegularExpression regexName("^[^0-9][a-zA-Z0-9_]{2,16}$");
    //(?=.*[A-Za-z])：正则表达式预查，在这里表示要求字符串中至少包含一个字母（大小写不限）。
    static QRegularExpression regexPwd("^(?=.*[A-Za-z])(?=.*\\d)[\\w]{4,32}$");


    if(strName.isEmpty()||strPwd.isEmpty())
    {
        QMessageBox::critical(this,"注册","注册失败：用户名或密码不能为空！");
    }
    else if(!regexName.match(strName).hasMatch())
    {
        QMessageBox::critical(this,"注册","注册失败：用户名不能以数字开头，且至少两个字符！");
    }
    else if(!regexPwd.match(strPwd).hasMatch())
    {
        QMessageBox::critical(this,"注册","注册失败：密码必须包含至少一个数字和一个字符，且至少4个字符！");
    }
    else
    {
        PDU *pdu=mkPDU(0);//没有msg
        pdu->uiMsgType=ENUM_MSG_REGIST_REQUEST;
        // 都放到caData
        strPwd=hashString(strPwd);//加密
        // qDebug()<<strPwd;

        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        //发送给服务器
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

//注销按钮关联事件
void TCPClient::on_logoff_pb_clicked()
{
    QString strName=ui->name_le->text();
    QString strPwd=ui->pwd_le->text();

    if(strName.isEmpty()||strPwd.isEmpty())
    {
        QMessageBox::critical(this,"注销","注销失败：用户名或密码不能为空！");
    }
    else
    {
        PDU *pdu=mkPDU(0);//没有msg
        pdu->uiMsgType=ENUM_MSG_LOGOFF_REQUEST;
        // 都放到caData
        strPwd=hashString(strPwd);
        strncpy(pdu->caData,strName.toStdString().c_str(),32);
        strncpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
        //发送给服务器
        m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    }
}

