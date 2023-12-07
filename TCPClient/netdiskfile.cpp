#include "netdiskfile.h"
#include "tcpclient.h"//注：不能递归包含头文件
#include <QInputDialog>//输入框
#include <QMessageBox>//提示框
#include <QRegularExpression>//正则匹配
// #include <numeric> //累加操作，用于连接字符串

#include <QFileDialog>//文件对话框
#include <QBitArray>

#include "sharefile.h"

NetdiskFile::NetdiskFile(QWidget *parent)
    : QWidget{parent}
{
    this->setAttribute(Qt::WA_DeleteOnClose);

    m_pFileInfoHeaderL = new QLabel( "文件名\t\t文件类型\t\t创建时间\t\t文件大小");//显示表头信息
    m_pFileListW = new QListWidget;//显示文件列表

    //按钮如下
    m_pReturnPB = new QPushButton("返回");//返回
    m_pCreateDirPB = new QPushButton("新建目录");//新建目录
    m_pFlushDirPB = new QPushButton("刷新目录");//刷新目录
    m_pSearchFilePB = new QPushButton("搜索文件");//搜索文件

    m_pUploadFilePB = new QPushButton("上传文件");//上传文件
    m_pDownloadFilePB = new QPushButton("下载文件");//下载文件
    m_pShareFilePB = new QPushButton("分享文件");//分享文件
    m_pViewFilePB = new QPushButton("查看文件内容");//查看文件内容

    m_pViewTextFileTE = new QTextEdit;//显示文本文件内容
    m_pViewImageFileL = new QLabel;//显示图像文件内容

    m_pTimer=new QTimer;//定时器

    //垂直布局
    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pFlushDirPB);
    pDirVBL->addWidget(m_pSearchFilePB);

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUploadFilePB);
    pFileVBL->addWidget(m_pDownloadFilePB);
    pFileVBL->addWidget(m_pShareFilePB);
    pFileVBL->addWidget(m_pViewFilePB);

    QVBoxLayout *pRightVBL = new QVBoxLayout;
    pRightVBL->addWidget(m_pFileInfoHeaderL);
    pRightVBL->addWidget(m_pFileListW);

    //水平布局
    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addLayout(pRightVBL);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    //关联信号槽
    connect(m_pCreateDirPB,SIGNAL(clicked(bool)),this,SLOT(createDir()));//新建目录
    connect(m_pFlushDirPB,SIGNAL(clicked(bool)),this,SLOT(flushDir()));//刷新目录
    connect(m_pFileListW,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(enterDir()));//双击进入目录。。
    connect(m_pReturnPB,SIGNAL(clicked(bool)),this,SLOT(returnDir()));//返回父目录

    connect(m_pUploadFilePB,SIGNAL(clicked(bool)),this,SLOT(uploadFile()));//上传文件
    connect(m_pTimer,&QTimer::timeout,this,&NetdiskFile::uploadFileData);//上传文件内容
    connect(m_pDownloadFilePB,&QPushButton::clicked,this,&NetdiskFile::downLoadFile);//下载文件

    // connect(m_pFileListW,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));//右键菜单
    this->m_pFileListW->setContextMenuPolicy(Qt::CustomContextMenu);//表示自定义上下文菜单策略。
    connect(m_pFileListW, &QListWidget::customContextMenuRequested, this, &NetdiskFile::showContextMenu);//右键关联自定义菜单

    connect(m_pShareFilePB,SIGNAL(clicked(bool)),this,SLOT(shareFile()));//分享文件

}

void NetdiskFile::createDir()//创建目录
{
    QString dirName=QInputDialog::getText(this,"新建目录","目录名：");
    static QRegularExpression regexDirName("^[A-Za-z0-9_-]{1,32}$");//匹配只包含英文字母、下划线、短划线和数字的字符串，长度为1-32
    if(dirName.isEmpty())
    {
        QMessageBox::warning(this,"新建目录","目录名不能为空");
        return;
    }
    else if(!regexDirName.match(dirName).hasMatch())
    {
        QMessageBox::warning(this,"新建目录","目录名中包含非法字符，注：目录名只能包含英文字母、下划线、短划线和数字，且长度不能超过32！");
        return;
    }
    QString strName=TCPClient::getInstance().getLonginName();
    QString strCurrentPath=TCPClient::getInstance().getCurrentPath();

    PDU *pdu=mkPDU(strCurrentPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_CREATE_DIR_REQUEST;
    memcpy(pdu->caData,strName.toStdString().c_str(),strName.size());//用户名
    memcpy(pdu->caData+32,dirName.toStdString().c_str(),dirName.size());//目录名

    memcpy((char*)pdu->caMsg,strCurrentPath.toStdString().c_str(),strCurrentPath.size());//当前路径
    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void NetdiskFile::flushDir()//刷新目录
{
    QString strCurrentPath=TCPClient::getInstance().getCurrentPath();
    PDU *pdu=mkPDU(strCurrentPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_FLUSH_DIR_REQUEST;
    strncpy((char*)(pdu->caMsg),strCurrentPath.toStdString().c_str(),strCurrentPath.size());
    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void NetdiskFile::enterDir()//双击目录条目，进入目录，并显示其中文件
{
    QString strDirItem=m_pFileListW->currentItem()->text();
    QStringList strList=strDirItem.split("\t");
    if(strList[1]!="[DIR]") return;//不是目录没反应
    //是目录再向服务器请求，打开目录，获取该目录下的文件信息

    QString strCurrentPaht=TCPClient::getInstance().getCurrentPath()+"/"+strList[0];//请求进入的目录

    PDU *pdu=mkPDU(strCurrentPaht.size()+1);
    pdu->uiMsgType=ENUM_MSG_FLUSH_DIR_REQUEST;//和刷新目录一样的请求，只不过换了目录

    strcpy((char*)(pdu->caMsg),strCurrentPaht.toStdString().c_str());
    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void NetdiskFile::returnDir()//按钮返回上一级目录，和进入目录类似
{
    //首先获取当前目录
    QString strPath=TCPClient::getInstance().getCurrentPath();
    // QStringList strDirOrderList=strPath.split("/");

    // if(strDirOrderList[strDirOrderList.size()-1]==TCPClient::getInstance().getLonginName())
    if(strPath=="./user/"+TCPClient::getInstance().getLonginName())//这样更合理
    {
        QMessageBox::information(this,"返回","已达根目录，不能再返回！");
        return;
    }

    //累加操作：[begin,end]范围，""初始值，操作（lambda表达式）
    // QString result=std::accumulate(strDirOrderList.begin(), strDirOrderList.end()-1, QString(),
    //                            [](const QString &a, const QString &b) {
    //                                return a.isEmpty() ? b : a + "/" + b;
    //                            });

    int index=strPath.lastIndexOf('/');//秀儿，直接找最后一个/，然后删除后面的。
    strPath.remove(index,strPath.size()-index);

    qDebug()<<strPath;
    PDU *pdu=mkPDU(strPath.size()+1);//关于少加一个字节，则找不到该目录了。。
    pdu->uiMsgType=ENUM_MSG_FLUSH_DIR_REQUEST;

    strcpy((char*)pdu->caMsg,strPath.toStdString().c_str());
    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void NetdiskFile::deleteFile()//删除文件或目录
{
    QString strCurrentPath = TCPClient::getInstance().getCurrentPath();
    QListWidgetItem *pItem = m_pFileListW->currentItem();
    if(NULL==pItem)
    {
        QMessageBox::warning(this,"删除文件","请选择要删除的文件！");
        return;
    }
    QString strDeleteName=pItem->text();
    QStringList fileInfoList=strDeleteName.split("\t");
    if(fileInfoList[1]=="[DIR]")
    {
        int result=QMessageBox::information(this,"删除文件",QString("%1 是目录，确定要删除吗？").arg(fileInfoList[0]), QMessageBox::Yes,QMessageBox::No);
        if(result!=QMessageBox::Yes)    return;
    }

    PDU *pdu=mkPDU(strCurrentPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_DELETE_FILE_REQUEST;//删除文件请求
    strcpy(pdu->caData,fileInfoList[0].toStdString().c_str());
    memcpy((char*)pdu->caMsg,strCurrentPath.toStdString().c_str(),strCurrentPath.size());

    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void NetdiskFile::renameFile()//重命名文件或目录
{
    QString strCurrentPath = TCPClient::getInstance().getCurrentPath();
    QListWidgetItem *pItem = m_pFileListW->currentItem();
    if(NULL==pItem)
    {
        QMessageBox::warning(this,"重命名文件","请选择要重命名的文件！");
        return;
    }
    QString strOldName=pItem->text();
    QStringList fileInfoList=strOldName.split("\t");
    QString strNewName=QInputDialog::getText(this,"重命名文件","重命名为：");
    if(strNewName.isEmpty())
    {
        QMessageBox::warning(this,"重命名","新文件名不能为空！");
        return;
    }
    PDU *pdu=mkPDU(strCurrentPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_RENAME_FILE_REQUEST;//重命名文件请求
    strcpy(pdu->caData,fileInfoList[0].toStdString().c_str());//旧名字
    strcpy(pdu->caData+32,strNewName.toStdString().c_str());//新名字
    memcpy((char*)pdu->caMsg,strCurrentPath.toStdString().c_str(),strCurrentPath.size());//父目录路径

    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void NetdiskFile::createFile()//新建文件
{
    QString fileName=QInputDialog::getText(this,"新建文件","文件名：");
    if(fileName.isEmpty())
    {
        QMessageBox::warning(this,"新建文件","文件名不能为空");
        return;
    }

    QString strCurrentPath=TCPClient::getInstance().getCurrentPath();

    PDU *pdu=mkPDU(strCurrentPath.size()+1);

    pdu->uiMsgType=ENUM_MSG_CREATE_FILE_REQUEST;
    memcpy(pdu->caData,fileName.toStdString().c_str(),fileName.size());//目录名

    memcpy((char*)pdu->caMsg,strCurrentPath.toStdString().c_str(),strCurrentPath.size());//当前路径
    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}


void NetdiskFile::uploadFile()//上传文件
{
    QString strCurrentPath=TCPClient::getInstance().getCurrentPath();//当前网盘路径
    QString strFilePath=QFileDialog::getOpenFileName();//获得上传本地文件的绝对路径
    this->m_strUploadFile=strFilePath;

    QFile file(strFilePath);
    qint64 fileSize=file.size();//获取文件大小，用于验证文件是否上传成功

    if(strFilePath.isEmpty())
    {
        QMessageBox::warning(this,"上传文件","没有选择上传的文件，上传失败！");
        return;
    }
    //提取文件名
    int index=strFilePath.lastIndexOf('/');
    QString strFileName=strFilePath.right(strFilePath.size()-index-1);//从右取，不需要'/',故减1
    qDebug()<<strFileName;

    PDU *pdu=mkPDU(strCurrentPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_UPLOAD_FILE_REQUEST;
    // sprintf(pdu->caData,"%s %11d",strFileName.toStdString().c_str(),fileSize);//文件名，文件大小
    memcpy(pdu->caData,strFileName.toStdString().c_str(),strFileName.size());

    QString fileSizeStr = QString::number(fileSize);
    memcpy(pdu->caData+32,fileSizeStr.toStdString().c_str(),fileSizeStr.size());

    memcpy((char*)(pdu->caMsg),strCurrentPath.toStdString().c_str(),strCurrentPath.size());

    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;

    m_pTimer->start(1000);

}

void NetdiskFile::uploadFileData()//上传文件内容
{
    m_pTimer->stop();

    QFile file(this->m_strUploadFile);

    qDebug()<<file.fileName();

    //只读的方式打开文件
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"上传文件","无法打开文件，上传失败！");
        return;
    }
    // QByteArray byteArray = file.readAll();

    // qint64 res=TCPClient::getInstance().getTcpSocket().write(byteArray,byteArray.size());
    // qDebug()<<res;//输出读到的文件内容

    char *pBuffer=new char[4096];
    qint64 result=0;//读取到的实际字节数
    while (true)
    {
        result=file.read(pBuffer,4096);//实际读取的数据
        if(result>0 && result<=4096)
        {
            TCPClient::getInstance().getTcpSocket().write(pBuffer,result);
            // qDebug()<<QString(pBuffer);
        }
        else if(0==result)
        {
            break;
        }
        else
        {
            QMessageBox::warning(this,"上传文件","读取文件发生错误，上传失败！");
            break;
        }
    }
    file.close();//文件已经读取完毕
    delete []pBuffer;
}

void NetdiskFile::downLoadFile()//下载文件
{
    QString strCurrentPath = TCPClient::getInstance().getCurrentPath();
    QListWidgetItem *pItem = m_pFileListW->currentItem();
    if(NULL==pItem)
    {
        QMessageBox::warning(this,"下载文件","请选择要下载的文件！");
        return;
    }
    QString strDownloadFileName=pItem->text();//将下载的文件
    QStringList fileInfoList=strDownloadFileName.split("\t");
    if(fileInfoList[1]=="[DIR]")
    {//提示
        QMessageBox::information(this,"下载文件",QString("%1 是目录，暂时不能下载目录").arg(fileInfoList[0]));
        return;
    }

    // QString strSaveFilePath = QFileDialog::getSaveFileName();//获取本机保存文件的路径,no不是文件，是文件夹
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), QDir::homePath());
    if(folderPath.isEmpty())
    {
        QMessageBox::warning(this,"下载文件","请指定要保存的文件路径！");
        this->m_strSaveFilePath.clear();
        return;//没有选择下载文件的保存路径，不予下载文件
    }
    this->m_strSaveFilePath=folderPath+"/"+fileInfoList[0];//整个文件下载至本地的绝对路径

    //封装pdu
    PDU *pdu=mkPDU(strCurrentPath.size()+1);
    pdu->uiMsgType=ENUM_MSG_DOWNLOAD_FILE_REQUEST;
    strcpy(pdu->caData,fileInfoList[0].toStdString().c_str());//文件名
    memcpy((char*)pdu->caMsg,strCurrentPath.toStdString().c_str(),strCurrentPath.size());//路径

    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void NetdiskFile::shearFile()//剪切文件
{
    QString strCurrentPath = TCPClient::getInstance().getCurrentPath();
    QListWidgetItem *pItem = m_pFileListW->currentItem();//不存在选空的情况

    QString strDownloadFileName=pItem->text();//将剪切的文件
    QStringList fileInfoList=strDownloadFileName.split("\t");

    this->m_strMoveFile=strCurrentPath+"/"+fileInfoList[0];//剪切文件

}

void NetdiskFile::pasteFile()//粘贴文件
{
    QString strCurrentPath = TCPClient::getInstance().getCurrentPath();
    QStringList fileList= m_strMoveFile.split('/');
    m_strTargetFile=strCurrentPath+"/"+fileList[fileList.size()-1];//目标文件

    PDU *pdu=mkPDU(m_strMoveFile.size()+m_strTargetFile.size()+2);
    pdu->uiMsgType=ENUM_MSG_MOVE_FILE_REQUEST;
    sprintf(pdu->caData,"%d %d",int(m_strMoveFile.size()),int(m_strTargetFile.size()));//文件路径大小存入caData中

    memcpy((char*)pdu->caMsg,m_strMoveFile.toStdString().c_str(),m_strMoveFile.size());
    memcpy((char*)pdu->caMsg+(m_strMoveFile.size()+1),m_strTargetFile.toStdString().c_str(),m_strTargetFile.size());

    TCPClient::getInstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;

    m_strMoveFile.clear();//清空，粘贴文件只能执行一次
}


void NetdiskFile::showContextMenu(const QPoint &pos) //添加菜单条目
{
    QPoint globalPos=this->m_pFileListW->mapToGlobal(pos);//从QListWidget的本地坐标pos转换为全局坐标，确保右键菜单在鼠标指针的正确位置显示
    QListWidgetItem *selectedItem=this->m_pFileListW->itemAt(pos);//将检查右键点击的位置是否在列表视图的项目范围内，返回指向被选中项目的指针。如果指针为nullptr，则意味着右键点击不是在任何项目上，因此右键菜单不应该显示。
    qDebug() << "Custom context menu requested!";
    //右键选择listwidget条目时，弹出菜单
    if(selectedItem)
    {
        QMenu *m_pMenu=new QMenu(this);
        QAction *pOpenACT=new QAction("打开",this);
        QAction *pDownloadACT=new QAction("下载",this);
        QAction *pShareACT=new QAction("分享",this);
        QAction *pCopyACT=new QAction("复制",this);
        QAction *pShearACT=new QAction("剪切",this);

        QAction *pRenameACT=new QAction("重命名",this);
        QAction *pDeleteACT=new QAction("删除",this);
        QAction *pCancelACT=new QAction("取消",this);

        //关联槽函数
        connect(pDeleteACT,&QAction::triggered,this,&NetdiskFile::deleteFile);//删除
        connect(pRenameACT,&QAction::triggered,this,&NetdiskFile::renameFile);//重命名
        connect(pShearACT,&QAction::triggered,this,&NetdiskFile::shearFile);//剪切

        m_pMenu->addAction(pOpenACT);
        m_pMenu->addAction(pDownloadACT);
        m_pMenu->addAction(pShareACT);
        m_pMenu->addAction(pCopyACT);
        m_pMenu->addAction(pShearACT);

        m_pMenu->addAction(pRenameACT);
        m_pMenu->addAction(pDeleteACT);
        m_pMenu->addAction(pCancelACT);

        m_pMenu->exec(globalPos);
    }
    else
    {
        QMenu *m_pMenu2=new QMenu(this);
        QAction *pCreateACT=new QAction("新建文件",this);
        QAction *pPasteACT=new QAction("粘贴",this);
        QAction *pCancelACT2=new QAction("取消",this);

        //关联槽函数
        connect(pCreateACT,&QAction::triggered,this,&NetdiskFile::createFile);//新建文件
        connect(pPasteACT,&QAction::triggered,this,&NetdiskFile::pasteFile);//粘贴文件

        m_pMenu2->addAction(pCreateACT);       
        m_pMenu2->addAction(pPasteACT);
        m_pMenu2->addAction(pCancelACT2);
        m_pMenu2->exec(globalPos);
    }
}

void NetdiskFile::shareFile()//分享文件
{
    QListWidgetItem *pItem = m_pFileListW->currentItem();
    if(NULL==pItem)
    {
        QMessageBox::warning(this,"分享文件","请选择要分享的文件！");
        return;
    }

    QString strDownloadFileName=pItem->text();//将分享的文件
    QStringList fileInfoList=strDownloadFileName.split("\t");//文件和目录都可以分享

    this->m_strShareFile=fileInfoList[0];

    MyFriendList *pFriend=OperateWidget::getInstance().getFriendList();//好友界面
    QListWidget *pFriendList=pFriend->getAllFriendList();//好友列表
    ShareFile::getInstance().updateFriend(pFriendList);//分享文件，跳出可以分享的好友列表
    if(ShareFile::getInstance().isHidden())
    {
        ShareFile::getInstance().show();
    }
}

QString NetdiskFile::getShareFileName()
{
    return this->m_strShareFile;
}

void NetdiskFile::showDirContent(PDU *pdu)//刷新目录
{
    if (NULL == pdu) return;
    m_pFileListW->clear();
    char strPath[64];
    strcpy(strPath,(char*)pdu->caData);
    TCPClient::getInstance().setCurrentPath(strPath);//修改当前目录

    QString title = QString(strPath) + " 目录下";
    m_pFileListW->setToolTip(title);

    char caTmp[128]; // 为每次循环分配一个临时数组
    for (uint i = 0; i < pdu->uiMsgLen / 128; ++i)
    {
        memcpy(caTmp, (char *)(pdu->caMsg) + i * 128, 128);
        QString fileInfo = QString(caTmp); // 使用临时数组构建QString
        QStringList infoList = fileInfo.split("\t");
        QListWidgetItem *pItem = new QListWidgetItem;
        // 添加图标
        if (infoList[1] == "[DIR]")
        {
            pItem->setIcon(QIcon(QPixmap(":/images/dir.png")));
        } else {
            pItem->setIcon(QIcon(QPixmap(":/images/file.jpg")));
        }
        pItem->setText(fileInfo);
        m_pFileListW->addItem(pItem);
        qDebug() << caTmp;
        // delete pItem; // 删除QListWidgetItem对象
    }
}

void NetdiskFile::setIsDownload(bool status)//设置是否为下载文件状态
{
    this->m_bIsDownload=status;
}

bool NetdiskFile::getIsDownload()//获得状态
{
    return this->m_bIsDownload;
}


void NetdiskFile::recvFileData()//接收下载文件的数据流
{
    //保存文件，文件大小，已下载的大小
    QFile file(this->m_strSaveFilePath);
    if (!file.open(QIODevice::WriteOnly))//打开失败
    {
        QMessageBox::warning(this,"下载文件","保存文件打开失败，无法下载");
    }
    QByteArray buffer= TCPClient::getInstance().getTcpSocket().readAll();
    file.write(buffer,buffer.size());//写文件内容
    this->m_iRecved+=buffer.size();//已写文件大小
    if(this->m_iDownloadFileSize==this->m_iRecved)
    {
        //写入完成，关闭文件,重置属性
        file.close();
        this->m_iDownloadFileSize=0;
        this->m_iRecved=0;
        this->m_bIsDownload=false;
        QMessageBox::information(this,"下载文件","文件下载完成！");
    }
    if(this->m_iDownloadFileSize<this->m_iRecved)
    // else
    {
        file.close();
        this->m_iDownloadFileSize=0;
        this->m_iRecved=0;
        this->m_bIsDownload=false;
        QMessageBox::information(this,"下载文件","文件下载失败！");
    }
}

