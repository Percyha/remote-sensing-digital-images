#ifndef MYIMAGEREG_H
#define MYIMAGEREG_H

#include <QLabel>

class myimagereg : public QLabel
{
    Q_OBJECT
public:
    explicit myimagereg(QWidget *parent = nullptr);
    //鼠标进入事件
    void enterEvent(QEnterEvent *event);

    //鼠标离开事件
    void leaveEvent(QEvent *);
};

#endif // MYIMAGEREG_H
