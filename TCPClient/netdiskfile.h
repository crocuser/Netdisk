#ifndef NETDISKFILE_H
#define NETDISKFILE_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include "protocol.h"
#include <QTimer>

//右键菜单需要
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QContextMenuEvent>
#include <QPoint>

class NetdiskFile : public QWidget
{
    Q_OBJECT
public:
    explicit NetdiskFile(QWidget *parent = nullptr);
    void showDirContent(PDU *pdu);

    void setIsDownload(bool status);//设置是否为下载文件状态
    bool getIsDownload();

    void recvFileData();//接收下载文件的内容

    qint64 m_iDownloadFileSize=0;//下载的文件的大小
    qint64 m_iRecved=0;//写入文件的大小

signals:

public slots:
    void createDir();//新建目录
    void flushDir();//刷新目录
    void enterDir();//进入目录
    void returnDir();//返回上一级目录
    void deleteFile();//删除文件或目录
    void renameFile();//重命名文件或目录
    void createFile();//新建文件
    void uploadFile();//上传文件
    void uploadFileData();//上传文件内容
    void downLoadFile();//下载文件
    void shearFile();//剪切文件
    void pasteFile();//粘贴文件

    void showContextMenu(const QPoint &pos); //在发生上下文菜单事件时自动调用的。

    void shareFile();//分享文件
    QString getShareFileName();//获取要分享的文件名


private:
    QLabel *m_pFileInfoHeaderL;//显示固定表头信息
    QListWidget *m_pFileListW;//显示文件列表

    //按钮如下
    QPushButton *m_pReturnPB;//返回
    QPushButton *m_pCreateDirPB;//新建目录
    QPushButton *m_pFlushDirPB;//刷新目录
    QPushButton *m_pSearchFilePB;//搜索文件

    QPushButton *m_pUploadFilePB;//上传文件
    QPushButton *m_pDownloadFilePB;//下载文件
    QPushButton *m_pShareFilePB;//分享文件
    QPushButton *m_pViewFilePB;//查看文件内容

    QTimer *m_pTimer;
    QString m_strUploadFile;//当前上传的文件

    QString m_strSaveFilePath;//要保存的文件路径
    bool m_bIsDownload=false;//是否处于下载文件的状态
    QString m_strShareFile;//要分享的文件

    QString m_strMoveFile;//移动文件
    QString m_strTargetFile;//目标文件

public:

    //查看文件内容
    QTextEdit *m_pViewTextFileTE;//显示文本文件
    QLabel *m_pViewImageFileL;//显示图像文件

};

#endif // NETDISKFILE_H
