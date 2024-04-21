#ifndef IMAGE_REGISTRATION_H
#define IMAGE_REGISTRATION_H

#include <QMainWindow>

#include <mygraphicscene.h>

#include <gdal_priv.h>
#include <operation.h>

#include <QGraphicsScene>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

namespace Ui {
class image_registration;
}

class image_registration : public QMainWindow
{
    Q_OBJECT

public:
    explicit image_registration(QWidget *parent = nullptr);
    ~image_registration();


    int s=1;

    operation * op;
    QGraphicsScene *scene;

    // 存储待配准影像、参考影像的的特征点 控件坐标
    QVector<QPointF> Geo_src;
    QVector<QPointF> Geo_ref;
    QVector<QPointF> Piexl_src;
    QVector<QPointF> Piexl_ref;
    // 地理坐标
    QVector<int> Geo_src_x;
    QVector<int> Geo_src_y;
    QVector<int> Geo_ref_x;
    QVector<int> Geo_ref_y;
    // 像素坐标
    // 地理坐标
    QVector<int> Piexl_src_x;
    QVector<int> Piexl_src_y;
    QVector<int> Piexl_ref_x;
    QVector<int> Piexl_ref_y;


    qreal  m_scaleValue;
    float m_scaleFactor;
    QVector<double> geo;

private slots:
    void on_actiondakai_triggered();

    void on_actionoperate_triggered();

    void srcPoints(QPointF pt); // 待配准影像设置信号接收槽
    void refPoints(QPointF pt); // 标准影像设置信号接收槽
protected:
    void wheelEvent(QWheelEvent *event);
    void ZoomIn();
    void ZoomOut();
    void ScaleImg(double factor);

private:
    Ui::image_registration *ui;
};
#endif // IMAGE_REGISTRATION_H
