#include "mygraphicscene.h"
#include <QDebug>

mygraphicscene::mygraphicscene(QWidget *parent) : QGraphicsView(parent)
{
    //设置鼠标追踪状态，默认是false
    setMouseTracking(true);
}

void mygraphicscene::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        QPointF po = event->position();
        emit mousePositionsUpdated(po);
    }
}

void mygraphicscene::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        QPointF po = event->position();
        emit mousePositionsUpdated(po);
    }
}

