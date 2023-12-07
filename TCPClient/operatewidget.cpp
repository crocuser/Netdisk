#include "operatewidget.h"
#include "tcpclient.h"

OperateWidget::OperateWidget(QWidget *parent)
    : QWidget{parent}
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowTitle(TCPClient::getInstance().getLonginName()+" 的世界");//登录用户名作为窗口标题

    m_pListW=new QListWidget(this);
    m_pListW->addItem("好友");
    m_pListW->addItem("文件");
    m_pListW->setFixedWidth(100);

    m_pMyFriendList = new MyFriendList;
    m_pNetdiskFile = new NetdiskFile;

    m_pStackedW = new QStackedWidget;
    m_pStackedW->addWidget(m_pMyFriendList);//默认选择第一个窗口
    m_pStackedW->addWidget(m_pNetdiskFile);


    //水平布局
    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pListW);
    pMain->addWidget(m_pStackedW);


    setLayout(pMain);

    //显示不同的窗口
    connect(m_pListW,SIGNAL(currentRowChanged(int)),m_pStackedW,SLOT(setCurrentIndex(int)));
}

OperateWidget &OperateWidget::getInstance()
{
    static OperateWidget instance;    
    return instance;
}

MyFriendList *OperateWidget::getFriendList()
{
    return m_pMyFriendList;
}

NetdiskFile *OperateWidget::getNetdisFile()
{
    return m_pNetdiskFile;
}
