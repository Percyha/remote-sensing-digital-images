#include "image_registration.h"
#include "ui_image_registration.h"

image_registration::image_registration(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::image_registration)
{
    ui->setupUi(this);
    op = new operation();
    scene = new QGraphicsScene();

    m_scaleValue = 1; // 初始化缩放比例

    ui->graphicsView->setMouseTracking(true);
    connect(ui->graphicsView_2, &mygraphicscene::mousePositionsUpdated, this, &image_registration::srcPoints); //待配准影像信号与槽绑定
    connect(ui->graphicsView, &mygraphicscene::mousePositionsUpdated, this, &image_registration::refPoints); //标准影像信号与槽绑定

    QLabel *statubar_Label = new QLabel("开发者：徐梓杰");
    statubar_Label->setMinimumWidth(200);
    statusBar()->addWidget(statubar_Label);
    QLabel *statubar_Label1 = new QLabel("开发日期：2023-11-21");
    statubar_Label1->setMinimumWidth(200);
    statusBar()->addWidget(statubar_Label1);
}

image_registration::~image_registration()
{
    delete ui;
}

void image_registration::on_actiondakai_triggered()
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
        s++;
    }
    else
    {
        op->Historm(ui->graphicsView_2->height());
        scene=op->myscene;
        ui->graphicsView_2->setScene(scene);
    }
}


void image_registration::on_actionoperate_triggered()
{
    // 存储待配准点的特征点
    op->handpeizhun(Geo_src_x,Geo_src_y,Geo_ref_x,Geo_ref_y,Piexl_src_x,Piexl_src_y,Piexl_ref_x,Piexl_ref_y,geo);
    ui->graphicsView_2->setScene(op->myscene);
}

void image_registration::srcPoints(QPointF pt)
{
    Geo_src.append(pt);
    Piexl_src_x.append(pt.x());
    Piexl_src_y.append(pt.y());
    ui->label_6->setText("控件坐标：（"+QString::number(pt.x())+","+QString::number(pt.y())+"）"); // 将鼠标获取的控件坐标显示在label标签中
    // 转成地理坐标
    double adfGeoTransform[6];
    op->rasterDataset->GetGeoTransform(adfGeoTransform);
    int GeoX=adfGeoTransform[0]+adfGeoTransform[1]*pt.x()+adfGeoTransform[2]*pt.y();
    int GeoY=adfGeoTransform[3]+adfGeoTransform[4]*pt.x()+adfGeoTransform[5]*pt.y();
    ui->label_8->setText("地理坐标:("+QString::number(GeoX)+","+QString::number(GeoY)+")");
    Geo_src_x.append(GeoX);
    Geo_src_y.append(GeoY);

    for(int i=0;i<6;i++)
    {
        geo.append(adfGeoTransform[i]);
    }
}

void image_registration::refPoints(QPointF pt)
{
    Geo_ref.append(pt);
    Piexl_ref_x.append(pt.x());
    Piexl_ref_y.append(pt.y());
    ui->label_5->setText("控件坐标：（"+QString::number(pt.x())+","+QString::number(pt.y())+"）"); // 将鼠标获取的控件坐标显示在label标签中
    // 转成地理坐标
    double adfGeoTransform[6];
    op->rasterDataset->GetGeoTransform(adfGeoTransform);
    int GeoX=adfGeoTransform[0]+adfGeoTransform[1]*pt.x()+adfGeoTransform[2]*pt.y();
    int GeoY=adfGeoTransform[3]+adfGeoTransform[4]*pt.x()+adfGeoTransform[5]*pt.y();
    ui->label_7->setText("地理坐标:("+QString::number(GeoX)+","+QString::number(GeoY)+")");
    Geo_ref_x.append(GeoX);
    Geo_ref_y.append(GeoY);
}

void image_registration::wheelEvent(QWheelEvent *event)
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

void image_registration::ZoomIn()
{
    ScaleImg(m_scaleValue);
}

void image_registration::ZoomOut()
{
    ScaleImg(m_scaleValue);
}

void image_registration::ScaleImg(double factor)
{
    m_scaleFactor = factor;

    QTransform *transform = new QTransform();
    transform->scale(m_scaleFactor, m_scaleFactor);
    ui->graphicsView->setTransform(*transform);
    ui->graphicsView_2->setTransform(*transform);
}


