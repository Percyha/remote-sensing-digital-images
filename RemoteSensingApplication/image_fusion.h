#ifndef IMAGE_FUSION_H
#define IMAGE_FUSION_H

#include <QMainWindow>
#include <QLabel>
#include <operation.h>
#include <QGraphicsScene>
#include <QWheelEvent>

namespace Ui {
class Image_Fusion;
}

class Image_Fusion : public QMainWindow
{
    Q_OBJECT

public:
    explicit Image_Fusion(QWidget *parent = nullptr);
    ~Image_Fusion();

    int s = 1;

    operation *op;
    QGraphicsScene *scene;

    qreal  m_scaleValue;
    float m_scaleFactor;

private slots:
    void on_actionopen_triggered();

    void on_actionBrovey_triggered();
    void on_actionIHS_triggered();

    void on_actionPCA_triggered();

protected:
    void wheelEvent(QWheelEvent *event);
    void ZoomIn();
    void ZoomOut();
    void ScaleImg(double factor);

private:
    Ui::Image_Fusion *ui;
};

#endif // IMAGE_FUSION_H
