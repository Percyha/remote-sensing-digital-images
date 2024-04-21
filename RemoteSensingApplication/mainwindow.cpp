#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    dw = new Dialog_Wait();
    ir = new image_registration();
    ifu = new Image_Fusion();

    plot=new QwtPlot();
    qwtscene=new QGraphicsScene();
    scene=new QGraphicsScene();

    m_standardItemModel = new QStandardItemModel();
    m_standardItemModel->setHorizontalHeaderLabels(QStringList(QStringLiteral("影像列表")));
    this->setMouseTracking(true);

    m_scaleValue = 1; // 初始化缩放比例
    // 以鼠标为中心缩放的效果
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    operateRaster=new operation();

    // statusbar添加控件
    window_Coordinates = new QLabel("View坐标:(-1,-1)");
    window_Coordinates->setMinimumWidth(200);
    statusBar()->addWidget(window_Coordinates);
    widget_Coordinates = new QLabel("原始坐标:(-1,-1)");
    widget_Coordinates->setMinimumWidth(200);
    statusBar()->addWidget(widget_Coordinates);
    Geo_Coordinates = new QLabel("地理坐标:(-1,-1)");
    Geo_Coordinates->setMinimumWidth(200);
    statusBar()->addWidget(Geo_Coordinates);
}

MainWindow::~MainWindow()
{
    delete ui;
}

#pragma region"打开遥感影像并显示其基本信息"{
void MainWindow::on_Action_OpenRaster_triggered()
{
    operateRaster->rasterDataset=NULL;
    operateRaster->rasterDataset = operateRaster->OpenRasterFile(); // 调用打开影像的函数

    // TreeView控件载入model
    ui->treeView->setModel(m_standardItemModel);
    // 展开数据
    ui->treeView->expandAll();
    addData2Treeview(operateRaster->rasterDataset->GetRasterCount());

}
#pragma endregion}

void MainWindow::addData2Treeview(int bandCount)
{
    // 创建根节点，抽象Item，并没有实际数据
    QStandardItem* itemRoot = m_standardItemModel->invisibleRootItem();
    QStandardItem* itemCam ;
    QList<QStandardItem*> camList;
    if(bandCount==3)
    {
        // 创建并添加Item的第一个子节点
        itemCam = new QStandardItem(operateRaster->ShowRasterInformation(operateRaster->rasterDataset)[0]);
        itemRoot->appendRow(itemCam);

        // 向第一个子节点itemCam添加子节点数据
        camList.append(new QStandardItem("band1"));
        camList.append(new QStandardItem("band2"));
        camList.append(new QStandardItem("band3"));
        itemCam->appendRows(camList);
    }
    else if(bandCount==4)
    {
        // 创建并添加Item的第一个子节点
        itemCam = new QStandardItem(operateRaster->ShowRasterInformation(operateRaster->rasterDataset)[0]);
        itemRoot->appendRow(itemCam);

        // 向第一个子节点itemCam添加子节点数据
        camList.append(new QStandardItem("band1"));
        camList.append(new QStandardItem("band2"));
        camList.append(new QStandardItem("band3"));
        camList.append(new QStandardItem("band4"));
        itemCam->appendRows(camList);
    }
    else
    {
        // 创建并添加Item的第一个子节点
        itemCam = new QStandardItem(operateRaster->ShowRasterInformation(operateRaster->rasterDataset)[0]);
        itemRoot->appendRow(itemCam);

        // 向第一个子节点itemCam添加子节点数据
        camList.append(new QStandardItem("band1"));
        itemCam->appendRows(camList);
    }
}

void MainWindow::ZoomIn()
{
    ScaleImg(m_scaleValue);
}

void MainWindow::ZoomOut()
{
    ScaleImg(m_scaleValue);
}

void MainWindow::ScaleImg(double factor)
{
    m_scaleFactor = factor;

    QTransform *transform = new QTransform();
    transform->scale(m_scaleFactor, m_scaleFactor);
    ui->graphicsView->setTransform(*transform);
    ui->graphicsView_2->setTransform(*transform);
    ui->graphicsView_3->setTransform(*transform);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //QGraphicsView坐标
    QPoint viewPoint = event->pos();
    int vpx=viewPoint.x()-ui->graphicsView->x();
    int vpy=viewPoint.y()-ui->graphicsView->y()-ui->menubar->height();
    window_Coordinates->setText("View坐标:("+QString::number(vpx)+","+QString::number(vpy)+")");
    // 原始的X坐标 = 鼠标在BMP上的X坐标 * （Dataset的宽度 / BMP的宽度）
    // 原始的Y坐标 = 鼠标在BMP上的Y坐标 * （Dataset的高度 / BMP的高度）
    int x=vpx/operateRaster->m_scaleFactor;
    int y=vpy/operateRaster->m_scaleFactor;
    widget_Coordinates->setText("原始坐标:("+QString::number(x)+","+QString::number(y)+")");
    double adfGeoTransform[6];
    operateRaster->rasterDataset->GetGeoTransform(adfGeoTransform);
    int GeoX=adfGeoTransform[0]+adfGeoTransform[1]*x+adfGeoTransform[2]*y;
    int GeoY=adfGeoTransform[3]+adfGeoTransform[4]*x+adfGeoTransform[5]*y;
    Geo_Coordinates->setText("地理坐标:("+QString::number(GeoX)+","+QString::number(GeoY)+")");
}

void MainWindow::wheelEvent(QWheelEvent *event)
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

#pragma region"直方图显示"{
void MainWindow::on_Action_ShowHistogram_triggered()
{
    operateRaster->show=false;

    operateRaster->currentIndex=ui->comboBox->currentIndex();

    // dw->open();
    QMessageBox msgBox;
    msgBox.setText("The document has been modified.");
    msgBox.show();

    operateRaster->Historm();
    plot= operateRaster->p1;
    plot->setFixedSize(ui->graphicsView_2->width(),ui->graphicsView_2->height());
    qwtscene->addWidget(plot);
    ui->graphicsView_2->setScene(qwtscene);

    plot->show();
}
#pragma endregion}

#pragma region"影像显示"{
void MainWindow::on_Action_ShowRaster_triggered()
{
    operateRaster->show=true;
    operateRaster->histormEqualization=false;
    operateRaster->liner=0;
    operateRaster->lvboo=false;
    operateRaster->dft=false;

    if(s==1)
    {
        operateRaster->Historm(ui->graphicsView->height());
        scene=operateRaster->myscene;
        ui->graphicsView->setScene(scene);
        s++;
    }
    else
    {
        operateRaster->Historm(ui->graphicsView_3->height());
        scene=operateRaster->myscene;
        ui->graphicsView_3->setScene(scene);
    }

    ui->textEdit->clear();
    RasterBasicInformation=operateRaster->ShowRasterInformation(operateRaster->rasterDataset);
    ui->textEdit->insertPlainText("影像存储位置："+RasterBasicInformation[1]+"\n");
    ui->textEdit->insertPlainText("影像大小:"+RasterBasicInformation[3]+"\n");
    ui->textEdit->insertPlainText("波段数:"+RasterBasicInformation[4]+"\n");
    ui->textEdit->insertPlainText("影像投影信息:"+RasterBasicInformation[5]+"\n");
}
#pragma endregion}

#pragma region"退出程序"{
void MainWindow::on_Action_Exit_triggered()
{
    this->close();
}
#pragma endregion}

#pragma region"影像增强"{

#pragma region"百分比拉伸"{
void MainWindow::on_action_Percentage_Stretch_triggered()
{
    operateRaster->liner=2;
    operateRaster->show=true;
    operateRaster->histormEqualization=false;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
}
#pragma endregion}

#pragma region"直方图均衡化"{
void MainWindow::on_action_Histogram_Equalization_triggered()
{
    operateRaster->histormEqualization=true;
    operateRaster->radiationCalibration=false;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
}
#pragma endregion}

#pragma region"均值滤波平滑"{
void MainWindow::on_action_Mean_Smoothing_triggered()
{
    operateRaster->lvboo=true;
    operateRaster->filterNum=1;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
    operateRaster->lvboo=false;
}
#pragma endregion}

#pragma region"sobel滤波"{
void MainWindow::on_action_Sobel_Gradient_triggered()
{
    operateRaster->lvboo=true;
    operateRaster->filterNum=2;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
    operateRaster->lvboo=false;
}
#pragma endregion}

#pragma region"Laplace滤波"{
void MainWindow::on_action_Laplace_Gradient_triggered()
{
    operateRaster->lvboo=true;
    operateRaster->filterNum=3;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
    operateRaster->lvboo=false;
}
#pragma endregion}

#pragma region"DFT变换"{
void MainWindow::on_actionDFT_2_triggered()
{
    operateRaster->dft=true;
    operateRaster->dftnum = -1;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
    operateRaster->dft=false;
}

void MainWindow::on_actionli_triggered()
{
    operateRaster->dft=true;
    operateRaster->dftnum = 1;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
    operateRaster->dft=false;
}


void MainWindow::on_actiongauss_triggered()
{
    operateRaster->dft=true;
    operateRaster->dftnum = 2;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
    operateRaster->dft=false;
}

void MainWindow::on_action_high_pass_triggered()
{
    operateRaster->dft=true;
    operateRaster->dftnum = 3;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
    operateRaster->dft=false;
}

#pragma endregion}

#pragma endregion}

#pragma region"影像配准"{

#pragma region"无人机影像粗校正"{
void MainWindow::on_action_Rough_Georeferencing_of_UAV_Imagery_triggered()
{
    // 无人机拍摄瞬间的参数
    float Phi=-2.4f;
    float Kappa=0.9f;
    float Omega=-5;                   // 成像时无人机的姿态角
    float b = 30.6137365f;            // 无人机拍摄瞬间纬度
    float l = 105.1847654f;           // 无人机拍摄瞬间经度
    float z = 823.136f;               // 无人机拍摄瞬间高程Z
    // 无人机相机型号参数 焦距、传感器（像片）参数
    float Camera_f=36;                // 无人机拍摄相机焦距
    float Camera_w=36;                // 传感器（像片）宽度
    float Camera_h=24;                // 传感器（像片）高度
    // 输出影像参数
    float outSize = 0.1f;              // 输出影像空间分辨率

    dw->show();
    operateRaster->ImageRegistration(b,l,z,Phi,Kappa,Omega,Camera_f,Camera_w,Camera_h,outSize);
    ui->graphicsView->setScene(operateRaster->myscene);
}
#pragma endregion}

#pragma region"手动配准"{
void MainWindow::on_action_Manual_Registration_triggered()
{
    ir -> show();
}
#pragma endregion}

#pragma region"特征点选取"{
void MainWindow::on_action_KeyPoints_triggered()
{
    if(ss==1)
    {
        operateRaster->n=1;
        scene=operateRaster->SIFT();
        ui->graphicsView_2->setScene(scene);
        ss++;
    }
    else
    {
        operateRaster->n=2;
        scene=operateRaster->SIFT();
        ui->graphicsView_3->setScene(scene);
    }
}
#pragma endregion}

#pragma region"FLANN暴力算法"{
void MainWindow::on_actionFLANN_triggered()
{
    scene=operateRaster->SIFT_FLANN();
    ui->graphicsView->setScene(scene);
}
#pragma endregion}

#pragma endregion}

#pragma region"辐射定标"{
void MainWindow::on_action_RadiometricCalibration_triggered()
{
    operateRaster->radiationCalibration=true;
    operateRaster->histormEqualization=false;
    operateRaster->dixingjiaozheng = false;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
}
void MainWindow::on_action_Terrain_Correction_triggered()
{
    operateRaster->dixingjiaozheng = true;
    operateRaster->radiationCalibration=true;
    operateRaster->histormEqualization=false;

    operateRaster->Historm(ui->graphicsView->height());
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
}
#pragma endregion}

#pragma region"影像融合"{
#pragma region"Brovey变换"{
void MainWindow::on_actionBrovey_triggered()
{
    ifu -> show();
}
#pragma endregion}

void MainWindow::on_actionIHS_triggered()
{
    ifu -> show();
}
#pragma endregion}

void MainWindow::on_actionPCA_triggered()
{
    ifu -> show();
}


void MainWindow::on_actionKmeans_triggered()
{
    operateRaster->KMEANS();
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
}


void MainWindow::on_actionHarris_triggered()
{
    operateRaster->HARRIS();
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
}


void MainWindow::on_actionMoravec_triggered()
{
    operateRaster->MORAVEC();
    scene=operateRaster->myscene;
    ui->graphicsView->setScene(scene);
}


void MainWindow::on_action_LineDetection_triggered()
{
    operateRaster->LineDetection();
    scene = operateRaster -> myscene;
    ui -> graphicsView -> setScene(scene);
}


void MainWindow::on_action_IterativeThresholdSegmentation_triggered()
{
    operateRaster->IterativeThresholdSegmentation();
    scene = operateRaster -> myscene;
    ui -> graphicsView -> setScene(scene);
}


void MainWindow::on_actionOSTU_triggered()
{
    operateRaster->OSTU();
    scene = operateRaster -> myscene;
    ui -> graphicsView -> setScene(scene);
}


void MainWindow::on_actionISODATA_triggered()
{
    operateRaster->ISODATA();
    scene = operateRaster -> myscene;
    ui -> graphicsView -> setScene(scene);
}


void MainWindow::on_actionhunhegaussmodel_triggered()
{
    operateRaster -> GaussianMixtureModel();
    scene = operateRaster -> myscene;
    ui -> graphicsView -> setScene(scene);
}


void MainWindow::on_actionMLC_triggered()
{
    operateRaster -> MLC();
    scene = operateRaster -> myscene;
    ui -> graphicsView -> setScene(scene);
}


void MainWindow::on_actionmindistance_triggered()
{
    operateRaster -> MinDistance();
    scene = operateRaster -> myscene;
    ui -> graphicsView -> setScene(scene);
}

void MainWindow::on_actionNDVI_triggered()
{
    operateRaster->NDVI();
    scene = operateRaster -> myscene;
    ui -> graphicsView -> setScene(scene);
}

