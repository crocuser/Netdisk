#include "sharefile.h"
#include "tcpclient.h"
#include "operatewidget.h"

ShareFile::ShareFile(QWidget *parent)
    : QWidget{parent}
{

    m_pSelectAllPB = new QPushButton("全选");//全选按钮
    m_pCancelSelectPB = new QPushButton("取消选择");//取消全选按钮

    m_pOKPB = new QPushButton("确定");//确定发送按钮
    m_pCancelPB = new QPushButton("取消");//取消发送按钮

    m_pShowSA = new QScrollArea;;//可滚动内容
    m_pFriendW = new QWidget;//展示在线好友列表

    m_pGroupVBL=new QVBoxLayout(m_pFriendW);//列表布局

    m_pButtonGroup = new QButtonGroup(m_pFriendW);//按钮组用于管理按钮
    m_pButtonGroup->setExclusive(false);//可多选

    //水平布局
    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectPB);

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    //垂直布局
    QVBoxLayout *pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pShowSA);
    pMainVBL->addLayout(pDownHBL);

    setLayout(pMainVBL);//设置界面布局

    connect(m_pCancelSelectPB,SIGNAL(clicked(bool)),this,SLOT(cancelSelect()));//关联取消选择按钮
    connect(m_pSelectAllPB,SIGNAL(clicked(bool)),this,SLOT(selectAll()));//全选
    connect(m_pOKPB,SIGNAL(clicked(bool)),this,SLOT(okShare()));//确认分享
    connect(m_pCancelPB,SIGNAL(clicked(bool)),this,SLOT(cancelShare()));//取消分享

}

ShareFile &ShareFile::getInstance()//获取单例
{
    static ShareFile instance;
    return instance;
}

void ShareFile::updateFriend(QListWidget *pFriendList)//更新好友列表，用于分享文件
{
    if(NULL==pFriendList)
    {
        return;
    }
    QAbstractButton *tmp=NULL;
    QList<QAbstractButton*> preFriendList=m_pButtonGroup->buttons();//获得所有按钮
    for(int i=0;i<preFriendList.size();i++)
    {
        tmp=preFriendList[i];
        m_pGroupVBL->removeWidget(tmp);//布局里删
        m_pButtonGroup->removeButton(tmp);//管理里删
        preFriendList.removeOne(tmp);//列表里删
        delete tmp;//释放内存
        tmp=NULL;
    }

    QCheckBox *pCB=NULL;//复选按钮
    for(int i=1;i<pFriendList->count();i++)
    {
        QStringList friendInfo=pFriendList->item(i)->text().split("\t");
        pCB=new QCheckBox(friendInfo[0]);
        m_pGroupVBL->addWidget(pCB);//布局按钮
        m_pButtonGroup->addButton(pCB);//管理按钮
    }
    m_pShowSA->setWidget(m_pFriendW);//展示好友列表
}

void ShareFile::cancelSelect()//取消选择
{
    QList<QAbstractButton*> cbList=m_pButtonGroup->buttons();//获得所有按钮
    for(int i=0;i<cbList.size();i++)
    {
        if(cbList[i]->isChecked())//如果已经选择，则取消选择
        {
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()//全选
{
    QList<QAbstractButton*> cbList=m_pButtonGroup->buttons();//获得所有按钮
    for(int i=0;i<cbList.size();i++)
    {
        if(!cbList[i]->isChecked())//如果没有选择，则选择
        {
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShare()//确认分享
{
    QString strName=TCPClient::getInstance().getLonginName();//分享者
    QString strCurrentPath=TCPClient::getInstance().getCurrentPath();//分享文件的路径
    QString strSharefileName=strCurrentPath+"/"+OperateWidget::getInstance().getNetdisFile()->getShareFileName();//分享的文件名

    QList<QAbstractButton*> cbList=m_pButtonGroup->buttons();//获得所有按钮
    int count=0;//人数
    QStringList personList;
    for(int i=0;i<cbList.size();i++)
    {
        if(cbList[i]->isChecked())
        {
            count++;
            personList.append(cbList[i]->text());
        }
    }

    PDU *pdu=mkPDU(32*count+strSharefileName.size()+1);
    pdu->uiMsgType=ENUM_MSG_SHARE_FILE_REQUEST;
    sprintf(pdu->caData,"%s %d",strName.toStdString().c_str(),count);//分享者，人数
    for(int i=0;i<count;i++)
    {
        memcpy((char*)(pdu->caMsg)+i*32,personList[i].toStdString().c_str(),personList[i].size());//被分享者人名
    }
    memcpy((char*)(pdu->caMsg)+count*32,strSharefileName.toStdString().c_str(),strSharefileName.size());//分享的文件的路径，含文件名
    qDebug()<<strSharefileName;

    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);//发送分享文件请求
    free(pdu);
    pdu=NULL;
}

void ShareFile::cancelShare()//取消分享
{
    this->hide();//隐藏窗口
}

