#ifndef SHAREFILE_H
#define SHAREFILE_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>//用于管理按钮组的类
#include <QScrollArea>//用于显示可滚动内容的控件
#include <QCheckBox>
#include <QListWidget>

class ShareFile : public QWidget
{
    Q_OBJECT
public:
    explicit ShareFile(QWidget *parent = nullptr);
    void test();
    static ShareFile &getInstance();
    void updateFriend(QListWidget *pFriendList);

signals:

public slots:
    void cancelSelect();//取消选择
    void selectAll();//全选

    void okShare();//确认
    void cancelShare();//取消

private:
    QPushButton *m_pSelectAllPB;//全选按钮
    QPushButton *m_pCancelSelectPB;//取消全选按钮

    QPushButton *m_pOKPB;//确定发送按钮
    QPushButton *m_pCancelPB;//取消发送按钮

    QScrollArea *m_pShowSA;//可滚动内容
    QWidget *m_pFriendW;//展示在线好友列表
    QButtonGroup *m_pButtonGroup;//按钮组用于管理按钮

    QVBoxLayout *m_pGroupVBL;//复选列表的布局
};

#endif // SHAREFILE_H
