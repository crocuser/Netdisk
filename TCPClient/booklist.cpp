#include "booklist.h"

BookList::BookList(QWidget *parent)
    : QWidget{parent}
{
    this->setAttribute(Qt::WA_DeleteOnClose);
}
