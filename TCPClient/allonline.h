#ifndef ALLONLINE_H
#define ALLONLINE_H

#include <QWidget>
#include "protocol.h"

namespace Ui {
class AllOnline;
}

class AllOnline : public QWidget
{
    Q_OBJECT

public:
    explicit AllOnline(QWidget *parent = nullptr);
    ~AllOnline();
    void showUser(PDU *pdu);

private slots:
    void on_add_friend_pb_clicked();

private:
    Ui::AllOnline *ui;

};

#endif // ALLONLINE_H
