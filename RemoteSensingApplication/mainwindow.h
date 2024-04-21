#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <dialog_wait.h>
#include <image_registration.h>
#include <image_fusion.h>
#include <mygraphicscene.h>

#include <QDialog>
#include <QMainWindow>
#include <QTextEdit>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QStandardItem>
#include <QTreeView>
#include <QCursor>
#include <QTransform>
#include <QWheelEvent>
#include <QLabel>
#include <QMessageBox>

#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include <operation.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QLabel *window_Coordinates; // 显示影像相对于窗口的坐标-窗口坐标
    QLabel *widget_Coordinates; // 显示影像相对于控件的坐标-控件坐标
    QLabel *Geo_Coordinates;    // 影像的地理坐标，根据6参数进行计算

    int s=1;
    int ss=1; // 用于sift算法
    float m_scaleFactor;
    QPoint lastEventCursorPos;
    qreal  m_scaleValue;

    operation *operateRaster;

    QGraphicsScene *scene;
    QGraphicsScene *qwtscene;
    QStandardItemModel* m_standardItemModel;

    QwtPlot *plot;
    QVector<QString> RasterBasicInformation;

protected:
    void addData2Treeview(int bandCount);
    void wheelEvent(QWheelEvent *event);
    void ZoomIn();
    void ZoomOut();
    void ScaleImg(double factor);
    void mouseMoveEvent(QMouseEvent *event);    //鼠标移动事件

private slots:
    void on_Action_Exit_triggered();

    void on_Action_OpenRaster_triggered();

    void on_Action_ShowHistogram_triggered();

    void on_Action_ShowRaster_triggered();

    void on_action_Percentage_Stretch_triggered();

    void on_action_Histogram_Equalization_triggered();

    void on_action_Mean_Smoothing_triggered();

    void on_action_Sobel_Gradient_triggered();

    void on_action_Laplace_Gradient_triggered();

    void on_actionDFT_2_triggered();

    void on_action_Rough_Georeferencing_of_UAV_Imagery_triggered();

    void on_action_Manual_Registration_triggered();

    void on_action_KeyPoints_triggered();

    void on_actionFLANN_triggered();

    void on_action_RadiometricCalibration_triggered();

    void on_actionBrovey_triggered();

    void on_actionIHS_triggered();

    void on_actionPCA_triggered();

    void on_actionKmeans_triggered();

    void on_actionHarris_triggered();

    void on_actionMoravec_triggered();

    void on_action_LineDetection_triggered();

    void on_action_IterativeThresholdSegmentation_triggered();

    void on_actionOSTU_triggered();

    void on_actionISODATA_triggered();

    void on_actionhunhegaussmodel_triggered();

    void on_actionMLC_triggered();

    void on_actionmindistance_triggered();

    void on_action_Terrain_Correction_triggered();


    void on_action_high_pass_triggered();

    void on_actionli_triggered();

    void on_actiongauss_triggered();

    void on_actionNDVI_triggered();

private:
    Ui::MainWindow *ui;
    Dialog_Wait *dw;
    image_registration *ir;
    Image_Fusion *ifu;
};
#endif // MAINWINDOW_H
