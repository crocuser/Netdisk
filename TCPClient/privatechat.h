#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include "protocol.h"

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(QWidget *parent = nullptr);
    ~PrivateChat();
    static PrivateChat &getInstance();

    void setChatName(QString strName);
    void updateMessageList(const PDU *pdu);

private slots:
    void on_sendMessage_pb_clicked();

private:
    Ui::PrivateChat *ui;
    QString m_strChatName;
    QString m_strLoginName;
};

#endif // PRIVATECHAT_H
