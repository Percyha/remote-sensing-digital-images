#include "image_fusion.h"
#include "ui_image_fusion.h"

Image_Fusion::Image_Fusion(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Image_Fusion)
{
    ui->setupUi(this);
    op = new operation();
    scene = new QGraphicsScene();

    m_scaleValue = 1; // 初始化缩放比例

    QLabel *statubar_Label = new QLabel("开发者：徐梓杰");
    statubar_Label->setMinimumWidth(200);
    statusBar()->addWidget(statubar_Label);
    QLabel *statubar_Label1 = new QLabel("开发日期：2023-11-21");
    statubar_Label1->setMinimumWidth(200);
    statusBar()->addWidget(statubar_Label1);
}

Image_Fusion::~Image_Fusion()
{
    delete ui;
}

void Image_Fusion::on_actionopen_triggered()
{
    op->rasterDataset=NULL;
    op->rasterDataset = op->OpenRasterFile(); // 调用打开影像的函数

    op->show=true;
    op->histormEqualization=false;
    op->liner=0;
    op->lvboo=false;
    op->dft=false;

    if(s==1)
    {
        op->Historm(ui->graphicsView->height());
        scene=op->myscene;
        ui->graphicsView->setScene(scene);
        op->lowRaster = op->rasterDataset;
        s++;
    }
    else
    {
        op->Historm(ui->graphicsView_2->height());
        scene=op->myscene;
        ui->graphicsView_2->setScene(scene);
        op->highRaster = op->rasterDataset;
    }
}


void Image_Fusion::on_actionBrovey_triggered()
{
    op->Brovey();
    ui->graphicsView->setScene(op->myscene);
}

void Image_Fusion::wheelEvent(QWheelEvent *event)
{
    // 滚轮向上滑动加上键盘的control键，放大图像
    if (event->angleDelta().y() > 0 && event->modifiers() & Qt::ControlModifier) // delta() > 0) 注明：delta(),新版的Qt已经将其废除，它的返回值相当于angleDelta()的返回值的y点坐标
    {
        m_scaleValue*=1.1;//每次放大10%
        ZoomIn();
    }
    // 滚轮向下滑动加上键盘的control键，缩小图像
    if (event->angleDelta().y() <0 && event->modifiers() & Qt::ControlModifier)// delta() < 0)
    {
        m_scaleValue*=0.9;//每次缩放10%
        ZoomOut();
    }
}

void Image_Fusion::ZoomIn()
{
    ScaleImg(m_scaleValue);
}

void Image_Fusion::ZoomOut()
{
    ScaleImg(m_scaleValue);
}

void Image_Fusion::ScaleImg(double factor)
{
    m_scaleFactor = factor;

    QTransform *transform = new QTransform();
    transform->scale(m_scaleFactor, m_scaleFactor);
    ui->graphicsView->setTransform(*transform);
    ui->graphicsView_2->setTransform(*transform);
}


void Image_Fusion::on_actionIHS_triggered()
{
    op->IHS();
    ui->graphicsView->setScene(op->myscene);
}


void Image_Fusion::on_actionPCA_triggered()
{
    op->PCA();
    ui->graphicsView->setScene(op->myscene);
}

