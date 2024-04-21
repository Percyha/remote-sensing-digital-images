#ifndef MYGRAPHICSCENE_H
#define MYGRAPHICSCENE_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>

class mygraphicscene : public QGraphicsView
{
    Q_OBJECT
public:
    explicit mygraphicscene(QWidget *parent = nullptr);

    QVector<QPointF> src;
    QVector<QPointF> pre;
    // 鼠标按下
    virtual void mousePressEvent(QMouseEvent *event);
    // 鼠标移动
    virtual void mouseMoveEvent(QMouseEvent *event);

signals:
    // 定义一个信号，在鼠标事件发生时发射
    void mousePositionsUpdated(QPointF positions);
};

#endif // MYGRAPHICSCENE_H
