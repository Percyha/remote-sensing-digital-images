#include "myimagereg.h"
#include <QDebug>

myimagereg::myimagereg(QWidget *parent) : QLabel(parent)
{

}

void myimagereg::enterEvent(QEnterEvent *event)
{
    qDebug()<<"鼠标进入";
}

void myimagereg::leaveEvent(QEvent *)
{
    qDebug()<<"鼠标离开";
}
