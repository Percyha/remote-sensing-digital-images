#include "operation.h"
using namespace cv;
operation::operation()
{
    GDALAllRegister();//注册驱动
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "YES");
    QLibrary lib( "Comctl32.dll" );
    lib.setLoadHints( QLibrary::ResolveAllSymbolsHint );
    lib.load();
}

#pragma region"打开遥感影像"{
GDALDataset *operation::OpenRasterFile()
{
    QString filepath=""; // 影像存储地址
    QFileDialog::Options options = QFileDialog::Options(QFileDialog::DontUseNativeDialog);
    filepath= QFileDialog::getOpenFileName(nullptr,QFileDialog::tr(""), ".",QFileDialog::tr("(*.tiff *.tif *.img *.jpg *.dat)"),nullptr,options);
    QFileInfo fileInfo(filepath);
    RasterInf.append(fileInfo.fileName()); // 获取到文件的名称，返回值为 “test.tif”
    RasterInf.append(filepath);
    // 打开影像
    rasterDataset=NULL;
    rasterDataset = (GDALDataset*) GDALOpen( filepath.toStdString().c_str(),GA_ReadOnly );
    if (rasterDataset == NULL)
    {
        QMessageBox::critical(0,QFileDialog::tr("Error!"),QFileDialog::tr("Can not open file %1").arg(filepath));
        return nullptr;
    }

    return rasterDataset;
}
#pragma endregion}

#pragma region"统计遥感影像基本信息"{
QVector<QString> operation::ShowRasterInformation(GDALDataset *poDataset)
{
    // 描述信息
    QString rasterInformation = poDataset->GetDriver()->GetDescription();
    RasterInf.append(rasterInformation);
    // 影像大小
    int rasterX = poDataset->GetRasterXSize();
    int rasterY = poDataset->GetRasterYSize();
    RasterInf.append(QString::number(rasterX)+"*"+QString::number(rasterY));
    // 波段数
    int rasterCount = poDataset->GetRasterCount();
    RasterInf.append(QString::number(rasterCount));
    // 投影信息
    const char* rasterProjInformation = poDataset->GetProjectionRef();
    RasterInf.append(rasterProjInformation);
    // 地理坐标信息 粗校正用得上
    double adfGeoTransform[6];
    poDataset->GetGeoTransform(adfGeoTransform);
    QString s;
    for(int i=0;i<6;i++)
    {
        if(i==5)
            s+=QString::number(adfGeoTransform[i]);
        else
            s+=QString::number(adfGeoTransform[i])+",";
    }
    RasterInf.append(s);
    // 波段信息：数据集中重要的信息，有波段尺寸、数据类型、颜色信息等。
    GDALRasterBand *poBand;
    for(int i=0;i<rasterCount;i++)
    {
        poBand = poDataset->GetRasterBand(i+1); // poBand为指向第i个波段的指针
        // 波段尺寸
        int bandX = poBand->GetXSize();
        int bandY = poBand->GetYSize();
        const char* bandType = GDALGetDataTypeName(poBand->GetRasterDataType()); // 数据类型
        const char* colorTypeName = GDALGetColorInterpretationName(poBand->GetColorInterpretation()); // 颜色信息
        RasterInf.append(QString::number(bandX)+","+QString::number(bandY)+","+bandType+","+colorTypeName);
    }

    return RasterInf;
}
#pragma endregion}

#pragma region"灰度值的一些统计：最大值、最小值、频数、平均值、方差等"{
void operation::Historm()
{
    // 首先分别读取RGB三个波段
    float * rBand = new float[iScaleWidth*iScaleHeight];
    float * gBand = new float[iScaleWidth*iScaleHeight];
    float * bBand = new float[iScaleWidth*iScaleHeight];

    QVector<QVector<float>> satatistics_Bands;

    if(currentIndex==1)
    {
        rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, rBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        gBand=rBand;
        bBand=rBand;
    }
    else if(currentIndex==2)
    {
        rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, gBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        rBand=gBand;
        bBand=gBand;
    }
    else if(currentIndex==3)
    {
        rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, bBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        rBand=bBand;
        gBand=bBand;
    }
    else
    {
        rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, rBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, gBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, bBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
    }

    // 统计各波段的灰度值的最大值、最小值、各灰度值的个数
    QVector<QVector<float>> Get_HisMaxMin={};
    QVector<float> Get_R_HisMaxMin;
    QVector<float> Get_G_HisMaxMin;
    QVector<float> Get_B_HisMaxMin;

    // 最大值、最小值
    Get_R_HisMaxMin=GetHistormMAXMin(rBand,iScaleWidth*iScaleHeight);
    Get_G_HisMaxMin=GetHistormMAXMin(gBand,iScaleWidth*iScaleHeight);
    Get_B_HisMaxMin=GetHistormMAXMin(bBand,iScaleWidth*iScaleHeight);
    Get_HisMaxMin.append(Get_R_HisMaxMin);
    Get_HisMaxMin.append(Get_G_HisMaxMin);
    Get_HisMaxMin.append(Get_B_HisMaxMin);

    // 平均值、方差
    QVector<QVector<float>> Get_HisAveSted;
    QVector<float> Get_R_HisAveSted;
    QVector<float> Get_G_HisAveSted;
    QVector<float> Get_B_HisAveSted;
    Get_R_HisAveSted=GetAverageSteddv(rBand,iScaleWidth*iScaleHeight);
    Get_G_HisAveSted=GetAverageSteddv(gBand,iScaleWidth*iScaleHeight);
    Get_B_HisAveSted=GetAverageSteddv(bBand,iScaleWidth*iScaleHeight);
    Get_HisAveSted.append(Get_R_HisAveSted);
    Get_HisAveSted.append(Get_G_HisAveSted);
    Get_HisAveSted.append(Get_B_HisAveSted);

    //float r_maxHis = Get_R_HisMaxMin.at(0);
    //float r_minHis = Get_R_HisMaxMin.at(1);
    //float g_maxHis = Get_G_HisMaxMin.at(0);
    //float g_minHis = Get_G_HisMaxMin.at(1);
    //loat b_maxHis = Get_B_HisMaxMin.at(0);
    //float b_minHis = Get_B_HisMaxMin.at(1);

    //    int band1_min=rasterDataset->GetRasterBand(1)->GetMinimum();
    //    int band1_max=rasterDataset->GetRasterBand(1)->GetMaximum();
    //    int band2_min=rasterDataset->GetRasterBand(2)->GetMinimum();
    //    int band2_max=rasterDataset->GetRasterBand(2)->GetMaximum();
    //    int band3_min=rasterDataset->GetRasterBand(3)->GetMinimum();
    //    int band3_max=rasterDataset->GetRasterBand(3)->GetMaximum();
    // 频数统计
    satatistics_Bands=Satatistics_Bands(rBand,gBand,bBand);

    p1=ShowHistorm(satatistics_Bands); // 绘制直方图
}

QVector<QVector<float> > operation::Satatistics_Bands(float * rBand,float * gBand,float * bBand)
{
    QVector<QVector<float>> satatistics_rBand;
    QVector<QVector<float>> satatistics_gBand;
    QVector<QVector<float>> satatistics_bBand;
    QVector<QVector<float>> satatistics_Bands;
    satatistics_rBand = SataticsBandsValue(rBand,iScaleWidth*iScaleHeight);
    satatistics_gBand = SataticsBandsValue(gBand,iScaleWidth*iScaleHeight);
    satatistics_bBand = SataticsBandsValue(bBand,iScaleWidth*iScaleHeight);
    satatistics_Bands.append(satatistics_rBand);
    satatistics_Bands.append(satatistics_gBand);
    satatistics_Bands.append(satatistics_bBand);

    return satatistics_Bands;
}
#pragma endregion}

#pragma region"读取各个波段的灰度值及其频数、最大值、最小值"{
void operation::Historm(double height)
{
    // 影像的高和宽
    imgWidth = rasterDataset->GetRasterBand(1)->GetXSize();
    imgHeight = rasterDataset->GetRasterBand(1)->GetYSize();
    // 缩放比例系数
    m_scaleFactor = height * 1.0 / imgHeight;
    // 显示区域的高和宽
    iScaleWidth = (int)(imgWidth * m_scaleFactor - 1);
    iScaleHeight = (int)(imgHeight * m_scaleFactor - 1);

    // 首先分别读取RGB三个波段
    float * rBand = new float[iScaleWidth*iScaleHeight];
    float * gBand = new float[iScaleWidth*iScaleHeight];
    float * bBand = new float[iScaleWidth*iScaleHeight];

    if(rasterDataset->GetRasterCount()>=3)
    {
        rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, rBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, gBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, bBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
    }
    else
    {
        rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, rBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, gBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
        rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, imgWidth, imgHeight, bBand, iScaleWidth, iScaleHeight, GDT_Float32, 0, 0);
    }

    if(show)
    {
        ShowRaster(rBand,gBand,bBand); // 传参，传递三个波段 用于显示影像而不显示直方图
    }
}
#pragma endregion}

#pragma region"统计灰度值的最大值、最小值"{
QVector<float> operation::GetHistormMAXMin(float *band, int total)
{
    QVector<float> maxmin;
    float maxHis=band[0];
    float minHis=band[0];
    for (int i = 0; i < total; i++)
    {
        if(band[i]>maxHis)
            maxHis=band[i];
        if(band[i]<minHis)
            minHis=band[i];
    }
    maxmin.append(maxHis);
    maxmin.append(minHis);

    return maxmin;
}
#pragma endregion}

#pragma region"求平均值和方差"{
QVector<float> operation::GetAverageSteddv(float *band, int total)
{
    QVector<float> value;

    int sum = 0;
    int sum1=0;
    float average=0;
    float steedv=0;

    // 平均值
    for(int i=0;i<total;i++)
    {
        sum+=band[i];
    }
    average=sum/total;

    // 方差
    for(int i=0;i<total;i++)
    {
        sum1+=qPow(average-band[i],2);
    }
    steedv=sum1/total;

    value.append(average);
    value.append(steedv);

    return value;
}
#pragma endregion}

#pragma region"统计各波段灰度值频数"{
QVector<QVector<float>> operation::SataticsBandsValue(float *band,int total)
{
    float tempHis=0;
    int sum=0;
    bool s=true;
    QVector<QVector<float>> S;
    QVector<float> satatistics;
    QVector<float> tempBand;
    //QVector<float> temp;
    // 核心代码 ： 将原数组一个一个的取出来，如果和新建的数组没有重复的话就开始统计数量，同时将这个数放入这个暂时的数组中，最后将这个灰度值对应的频数统计出来
    for (int i = 0; i < total; i++)
    {
        tempHis=band[i];
        for(int j = 0;j < tempBand.length();j++)
        {
            if(tempHis==tempBand[j])
            {
                s=false;
            }
        }
        if(s)
        {
            tempBand.append(tempHis);
            //temp.append(tempBand.length());
            for (int m = 0; m < total; m++)
            {
                if(band[m]==tempHis)
                {
                    sum=sum+1;
                }
            }
            satatistics.append(sum);
        }
        s=true;
        sum=0;
    }
    S.append(tempBand);
    S.append(satatistics);
    return S;
}
#pragma endregion}

#pragma region"QWT绘制直方图"{
QwtPlot * operation::ShowHistorm(QVector<QVector<float>> satatics)
{
    QwtPlot *p=new QwtPlot();
    p->setFixedSize(800,600);

    // 这里困扰了我很久，因为画出来的线和奇怪，通过反复的实验尝试，发现是将数组的点依次连接，所以需要将里面的灰度值从小到大重新排序，对应的频数跟着变化
    QVector<QVector<float>> update_R_BandHistorm;
    update_R_BandHistorm.append(satatics.at(0));
    update_R_BandHistorm.append(satatics.at(1));
    update_R_BandHistorm=PaiXu(update_R_BandHistorm);

    QVector<QVector<float>> update_G_BandHistorm;
    update_G_BandHistorm.append(satatics.at(2));
    update_G_BandHistorm.append(satatics.at(3));
    update_G_BandHistorm=PaiXu(update_G_BandHistorm);

    QVector<QVector<float>> update_B_BandHistorm;
    update_B_BandHistorm.append(satatics.at(4));
    update_B_BandHistorm.append(satatics.at(5));
    update_B_BandHistorm=PaiXu(update_B_BandHistorm);

    //获取最大灰度值，重绘y坐标轴
    GUIntBig max1=update_R_BandHistorm.at(1).at(0);
    GUIntBig max2=update_G_BandHistorm.at(1).at(0);
    GUIntBig max3=update_B_BandHistorm.at(1).at(0);

    for(int j=0;j<update_R_BandHistorm.at(1).length()-1;j++)
    {
        if(max1<update_R_BandHistorm.at(1).at(j+1))
            max1=update_R_BandHistorm.at(1).at(j+1);
    }
    for(int j=0;j<update_G_BandHistorm.at(1).length()-1;j++)
    {
        if(max2<update_G_BandHistorm.at(1).at(j+1))
            max2=update_G_BandHistorm.at(1).at(j+1);
    }
    for(int j=0;j<update_B_BandHistorm.at(1).length()-1;j++)
    {
        if(max3<update_B_BandHistorm.at(1).at(j+1))
            max3=update_B_BandHistorm.at(1).at(j+1);
    }

    //找到最大值重设y值
    GUIntBig temp=max1;
    if(max1<max2)
    {
        temp=max2;
        if(max2<max3)
        {
            temp=max3;
        }
    }
    else
    {
        if(max1<max3)
        {
            temp=max3;
        }
    }
    //new 曲线
    QwtPlotCurve * curve1=new QwtPlotCurve("Curve 1");
    QwtPlotCurve * curve2=new QwtPlotCurve("Curve 2");
    QwtPlotCurve * curve3=new QwtPlotCurve("Curve 3");
    p->setAxisScale(QwtPlot::yLeft,0,temp,temp/10);
    //将点绑定在线上
    curve1->setSamples(update_R_BandHistorm.at(0),update_R_BandHistorm.at(1));
    curve2->setSamples(update_G_BandHistorm.at(0),update_G_BandHistorm.at(1));
    curve3->setSamples(update_B_BandHistorm.at(0),update_B_BandHistorm.at(1));
    //设置划线颜色
    curve1->setPen(Qt::red,1);
    curve2->setPen(Qt::green,1);
    curve3->setPen(Qt::blue,1);
    //将线绑定在Qwt控件上
    curve1->attach(p);
    curve2->attach(p);
    curve3->attach(p);
    //qwtplot重绘
    p->replot();

    //1、将鼠标左键按下，窗体会显示按下位置的坐标；
    //2、按下鼠标右键并移动，实现平移效果；
    //3、按下鼠标左键，然后框选感兴趣的区域。则感兴趣区域被放大显示（想要恢复原状，只要按下鼠标右键即可）：
    m_zoomer=new QwtPlotZoomer(p->canvas());
    m_panner=new QwtPlotPanner(p->canvas());

    m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    m_zoomer->setZoomBase(true);
    m_panner->setMouseButton(Qt::RightButton);

    return p;
}
#pragma endregion}

#pragma region"将灰度值及其对应的频数升序排列"{
QVector<QVector<float>> operation::PaiXu(QVector<QVector<float>> bandHistorm)
{
    QVector<float> bandvalue; // 存储排序后的灰度值
    QVector<float> sat; // 存储排序后的频数
    GUIntBig min1=bandHistorm.at(0).at(0); // 初始化为第一个灰度值，作为当前最小值
    int mark=0; // 记录当前最小值的索引
    int len=bandHistorm.at(0).length(); // 输入数组的长度，即波段数长度
    // 遍历排序
    for(int j=0;j<len;j++)
    {
        for(int i=0;i<bandHistorm.at(0).length()-1;i++)
        {
            if(min1>bandHistorm.at(0).at(i+1))
            {
                min1=bandHistorm.at(0).at(i+1);
                mark=i+1;
            }
        }
        if(bandHistorm.at(0).length()>1)
        {
            bandvalue.append(min1); // 灰度值
            sat.append(bandHistorm.at(1)[mark]); // 灰度值对应出现的频数
            bandHistorm[0].erase(bandHistorm[0].begin()+(mark));
            bandHistorm[1].erase(bandHistorm[1].begin()+(mark));
            min1=bandHistorm.at(0).at(0);
            mark=0;
        }
        else
        {
            bandvalue.append(bandHistorm[0][0]);
            sat.append(bandHistorm[1][0]);
            break;
        }
    }

    QVector<QVector<float>> updateBandHistorm;
    updateBandHistorm.append(bandvalue);
    updateBandHistorm.append(sat);

    return updateBandHistorm;
}
#pragma endregion}

#pragma region"显示遥感影像"{
QGraphicsScene * operation::ShowRaster(float* r,float* g,float* b)
{
    //显示影像
    unsigned char *rBandUC, *gBandUC, *bBandUC;

    if(rasterDataset->GetRasterCount()>=3)
    {
        // 把三个通道的灰度值缩放到0-255之间
        r =Stretch255(r,rasterDataset->GetRasterBand(1),iScaleWidth*iScaleHeight);
        g =Stretch255(g,rasterDataset->GetRasterBand(2),iScaleWidth*iScaleHeight);
        b =Stretch255(b,rasterDataset->GetRasterBand(3),iScaleWidth*iScaleHeight);
    }
    else
    {
        // 把三个通道的灰度值缩放到0-255之间
        r =Stretch255(r,rasterDataset->GetRasterBand(1),iScaleWidth*iScaleHeight);
        g =Stretch255(g,rasterDataset->GetRasterBand(1),iScaleWidth*iScaleHeight);
        b =Stretch255(b,rasterDataset->GetRasterBand(1),iScaleWidth*iScaleHeight);
    }


    if(dft==true)
    {
        rBandUC = DFT(r,iScaleHeight,iScaleWidth);
        gBandUC = DFT(g,iScaleHeight,iScaleWidth);
        bBandUC = DFT(b,iScaleHeight,iScaleWidth);
//        gBandUC = rBandUC;
//        bBandUC = rBandUC;
    }
    else if(lvboo==true)
    {
        rBandUC = lvbo(r,iScaleWidth,iScaleHeight,iScaleWidth*iScaleHeight);
        gBandUC = lvbo(g,iScaleWidth,iScaleHeight,iScaleWidth*iScaleHeight);
        bBandUC = lvbo(b,iScaleWidth,iScaleHeight,iScaleWidth*iScaleHeight);
    }
    // 最大最小值拉伸
    else if(liner==0)
    {
        rBandUC = ImgSketch(r, iScaleWidth * iScaleHeight);
        gBandUC = ImgSketch(g, iScaleWidth * iScaleHeight);
        bBandUC = ImgSketch(b, iScaleWidth * iScaleHeight);
    }
    // 2%线性拉伸
    else if(liner==2)
    {
        rBandUC = ImgSketchPercent_2(r, iScaleWidth * iScaleHeight, rasterDataset->GetRasterBand(1)->GetNoDataValue());
        gBandUC = ImgSketchPercent_2(g, iScaleWidth * iScaleHeight, rasterDataset->GetRasterBand(2)->GetNoDataValue());
        bBandUC = ImgSketchPercent_2(b, iScaleWidth * iScaleHeight, rasterDataset->GetRasterBand(3)->GetNoDataValue());
    }
    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = rBandUC[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = gBandUC[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bBandUC[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);

    return myscene;
}
#pragma endregion}

#pragma region"将灰度值都缩放至0-255"{
float *operation::Stretch255(float *buffer,GDALRasterBand *currentBand,int bandSize)
{
    double max, min;
    double minmax[2];
    currentBand->ComputeRasterMinMax(1, minmax); // 这里也可以用自己上面写的函数，这里是直接方便一些
    min = minmax[0];
    max = minmax[1];

    for (int i = 0; i < bandSize; i++)
    {
        if (buffer[i] > max)
        {
            buffer[i] = 255;
        }
        else if (buffer[i] <= max&& buffer[i] >= min)
        {
            buffer[i] = static_cast<uchar>(255 - 255 * (max - buffer[i]) / (max - min));
        }
        else
        {
            buffer[i] = 0;
        }
    }
    return buffer;
}

#pragma endregion}

#pragma region"最大最小值拉伸"{
// 线性拉伸 图像值拉伸到0~255灰度级（打开影像后默认线性拉伸 与envi功能一致），最大最小值拉伸
unsigned char *operation::ImgSketch(float *buffer, int bandSize)
{
    unsigned char* resBuffer = new unsigned char[bandSize];

    if(radiationCalibration==true)
    {
        resBuffer = RadiationCalibration(buffer,bandSize);
    }
    else if(histormEqualization==false)
    {
        for (int i = 0; i < bandSize; i++)
            resBuffer[i] = static_cast<uchar>(buffer[i]);
    }
    else if(histormEqualization == true)
    {
        resBuffer = HistormEqualization(buffer,bandSize);
    }

    return resBuffer;
}
#pragma endregion}

#pragma region"2%线性拉伸"{
unsigned char *operation::ImgSketchPercent_2(float *buffer, int bandSize, double noValue)
{
    unsigned char* resBuffer = new unsigned char[bandSize];

    QVector<QVector<float>> band; // 每个灰度值的频数
    QVector<float> band_probability; // 每个灰度值出现的概率
    QVector<float> band_Cumu_probability; // 归一概率
    QVector<QVector<float>> resband; // 拉伸后的灰度值

    band=PaiXu(SataticsBandsValue(buffer,bandSize));
    float sum_pro=0;
    int min_band=0;
    int max_band=0;

    for(int i=0;i<band[0].length();i++)
    {
        float pro=band[1][i]/bandSize;
        sum_pro+=pro;
        band_probability.append(pro);
        band_Cumu_probability.append(sum_pro);
        if(abs(sum_pro-0.02)<0.0001)
        {
            min_band=band[0][i];
        }
        if(abs(sum_pro-0.98)<0.001)
        {
            max_band=band[0][i];
        }
    }

    resband.append(band[0]);
    resband.append(band_Cumu_probability);

    if (min_band <= noValue && noValue <= max_band)
    {
        min_band = 0;
    }
    //开始拉伸 对原影像的灰度值进行拉伸
    for (int i = 0; i < bandSize; i++)
    {
        if (buffer[i] > max_band)
        {
            resBuffer[i] = 255;
        }
        else if (buffer[i] <= max_band&& buffer[i] >= min_band)
        {
            resBuffer[i] = static_cast<uchar>(255*(buffer[i]-min_band)/(max_band-min_band));
        }
        else
        {
            resBuffer[i] = 0;
        }
    }

    if(histormEqualization == true && liner == 2)
    {
        // 均衡化的代码
        for (int i = 0; i < bandSize; i++)
        {
            for(int j=0;j < resband[0].length(); j++)
            {
                if(buffer[i]==resband[0][j])
                {
                    resBuffer[i]=static_cast<uchar>(255*resband[1][j]+0.5);
                }
            }
        }
    }

    return resBuffer;
}
#pragma endregion}

#pragma region"直方图均衡化"{
unsigned char* operation::HistormEqualization(float *buffer,int bandSize)
{
    unsigned char* resBuffer = new unsigned char[bandSize];
    QVector<QVector<float>> band; // 每个灰度值及其对应的频数，升序排序
    QVector<float> band_probability; // 每个灰度值出现的概率
    QVector<float> band_Cumu_probability; // 归一概率
    QVector<QVector<float>> resband; // 拉伸后的灰度值及其对应的累积频率

    band=PaiXu(SataticsBandsValue(buffer,bandSize));
    float sum_pro=0;

    // 计算归一概率
    for(int i=0;i<band[0].length();i++)
    {
        float pro=band[1][i]/bandSize;
        sum_pro+=pro;
        band_probability.append(pro);
        band_Cumu_probability.append(sum_pro);
    }

    resband.append(band[0]);
    resband.append(band_Cumu_probability);

    // 计算均衡化后的灰度值
    for (int i = 0; i < bandSize; i++)
    {
        for(int j=0;j < resband[0].length(); j++)
        {
            if(buffer[i]==resband[0][j])
            {
                resBuffer[i]=static_cast<uchar>(255*resband[1][j]+0.5);
            }
        }
    }
    return resBuffer;
}
#pragma endregion}

#pragma region"辐射定标"{
unsigned char* operation::RadiationCalibration(float *buffer, int bandSize)
{
    unsigned char* resBuffer = new unsigned char[bandSize];
//    QVector<float> maxmin;
//    maxmin=GetHistormMAXMin(buffer,bandSize);
//    double max=maxmin[0];
//    double min=maxmin[1];

    for (int i = 0; i < bandSize; i++)
    {
/*        resBuffer[i] = static_cast<uchar>(buffer[i]*(max-min)/255+min);*/ // 这里的话有点问题，就是本来是10bit的影像，然后先转换成了8bit的数据，那这样最大最小值分别就是255和0，用辐射定标的话就没有任何意义
        resBuffer[i] = static_cast<uchar>(buffer[i]*1.5+0.5);
        if (dixingjiaozheng)
          resBuffer[i] = static_cast<uchar>(resBuffer[i]*qCos(M_PI*30/180)/qCos(M_PI*10/180)); // 地形校正 简单的余弦校正方法 可以让用户自定义输入

    }

    return resBuffer;
}
#pragma endregion}

#pragma region"大气校正（还未完成）"{
void operation::AtmosphericCorrection()
{

}
#pragma endregion}

#pragma region"卷积运算之滤波器选择"{
unsigned char *operation::lvbo(float *buffer,int width,int height,int bandsize)
{
    unsigned char* resBuffer = new unsigned char[bandsize];
    QVector<QVector<float>>buffer2dimension; // 二维数组存储遥感影像
    QVector<QVector<float>>buffer2dimension_large; // 二维数组存储遥感影像 扩展了边缘

    // 初始化行和列
    buffer2dimension.resize(width);
    for(int j=0;j<buffer2dimension.size();j++)
    {
        buffer2dimension[j].resize(height);
    }

    buffer2dimension_large.resize(width+2);
    for(int j=0;j<buffer2dimension_large.size();j++)
    {
        buffer2dimension_large[j].resize(height+2);
    }

    // 一维数组转成二维数组
    int m=0;
    for(int i=0;i<width;i++)
    {
        for(int j=0;j<height;j++)
        {
            buffer2dimension[i][j]=buffer[m];
            m++;
        }
    }
    // 扩展边缘 赋值为0
    for(int i=0;i<width+2;i++)
    {
        for(int j=0;j<height+2;j++)
        {
            if(i==0)
            {
                buffer2dimension_large[i][j]=0;
            }
            else if(j==0)
            {
                buffer2dimension_large[i][j]=0;
            }
            else if(i==width+1)
            {
                buffer2dimension_large[i][j]=0;
            }
            else if(j==height+1)
            {
                buffer2dimension_large[i][j]=0;
            }
            else
            {
                buffer2dimension_large[i][j]=buffer2dimension[i-1][j-1];
            }
        }
    }
    // 滤波器的选择
    QVector<float> filter;
    QVector<float> filter1;
    switch (filterNum) {
    case 1:
        for(int i=0;i<9;i++)
            filter.append(0.11111f);
        break;
    case 2:
        filter.append(-1);
        filter.append(-2);
        filter.append(-1);
        filter.append(0);
        filter.append(0);
        filter.append(0);
        filter.append(1);
        filter.append(2);
        filter.append(1);

        filter1.append(-1);
        filter1.append(0);
        filter1.append(1);
        filter1.append(-2);
        filter1.append(0);
        filter1.append(2);
        filter1.append(-1);
        filter1.append(0);
        filter1.append(1);
        break;
    case 3:
        filter.append(0);
        filter.append(1);
        filter.append(0);
        filter.append(1);
        filter.append(-4);
        filter.append(1);
        filter.append(0);
        filter.append(1);
        filter.append(0);


        filter1.append(0);
        filter1.append(0);
        filter1.append(0);
        filter1.append(0);
        filter1.append(0);
        filter1.append(0);
        filter1.append(0);
        filter1.append(0);
        filter1.append(0);
        break;
    default:
        filter.append(1);
        filter.append(1);
        filter.append(1);
        filter.append(1);
        filter.append(1);
        filter.append(1);
        filter.append(1);
        filter.append(1);
        filter.append(1);
        break;
    }
    float *x = new float[width * height];
    int pp=0;
//    for(int j=1;j<width+1;j++)
//    {
//        for(int n=1;n<height+1;n++)
//        {
//            x[pp] = (buffer2dimension_large[j][n]*filter[4]+buffer2dimension_large[j][n+1]*filter[5]+buffer2dimension_large[j][n-1]*filter[3]
//                     +buffer2dimension_large[j+1][n]*filter[7]+buffer2dimension_large[j+1][n+1]*filter[8]+buffer2dimension_large[j+1][n-1]*filter[6]
//                     +buffer2dimension_large[j-1][n]*filter[1]+buffer2dimension_large[j-1][n+1]*filter[2]+buffer2dimension_large[j-1][n-1]*filter[0]);
//            pp++;
//        }
//    }
    if(filterNum == 1)
    {
        for(int j=1;j<width+1;j++)
        {
            for(int n=1;n<height+1;n++)
            {
                x[pp] = (buffer2dimension_large[j][n]*filter[4]+buffer2dimension_large[j][n+1]*filter[5]+buffer2dimension_large[j][n-1]*filter[3]
                         +buffer2dimension_large[j+1][n]*filter[7]+buffer2dimension_large[j+1][n+1]*filter[8]+buffer2dimension_large[j+1][n-1]*filter[6]
                         +buffer2dimension_large[j-1][n]*filter[1]+buffer2dimension_large[j-1][n+1]*filter[2]+buffer2dimension_large[j-1][n-1]*filter[0]);
                pp++;
            }
        }
    }

    float *x1 = new float[width * height];
    int pp0 = 0;
    if(filterNum == 2 || filterNum == 3)
    {
        for(int j=1;j<width+1;j++)
        {
            for(int n=1;n<height+1;n++)
            {
                x[pp0] = (buffer2dimension_large[j][n]*filter[4]+buffer2dimension_large[j][n+1]*filter[7]+buffer2dimension_large[j][n-1]*filter[1]
                         +buffer2dimension_large[j+1][n]*filter[5]+buffer2dimension_large[j+1][n+1]*filter[8]+buffer2dimension_large[j+1][n-1]*filter[2]
                         +buffer2dimension_large[j-1][n]*filter[3]+buffer2dimension_large[j-1][n+1]*filter[6]+buffer2dimension_large[j-1][n-1]*filter[0]);

                x1[pp0] = (buffer2dimension_large[j][n]*filter1[4]+buffer2dimension_large[j][n+1]*filter1[7]+buffer2dimension_large[j][n-1]*filter1[1]
                           +buffer2dimension_large[j+1][n]*filter1[5]+buffer2dimension_large[j+1][n+1]*filter1[8]+buffer2dimension_large[j+1][n-1]*filter1[2]
                           +buffer2dimension_large[j-1][n]*filter1[3]+buffer2dimension_large[j-1][n+1]*filter1[6]+buffer2dimension_large[j-1][n-1]*filter1[0]);
                pp0++;
            }
        }

        for(int i = 0;i < width * height;i++)
        {
            x[i] = qSqrt(qPow(x[i],2) + qPow(x1[i],2));
        }
    }
//    QVector<float> maxmin = GetHistormMAXMin(x,width * height);

//    for (int i = 0; i < width * height; i++)
//    {
//        if (x[i] > maxmin[0])
//        {
//            x[i] = 255;
//        }
//        else if (x[i] <= maxmin[0] && x[i] >= maxmin[1])
//        {
//            x[i] = 255 - 255 * (maxmin[0] - x[i]) / (maxmin[0] - maxmin[1]);
//        }
//        else
//        {
//            x[i] = 0;
//        }
//    }
    // 均值滤波
    int p=0;
    for(int j=1;j<width+1;j++)
    {
        for(int n=1;n<height+1;n++)
        {
            resBuffer[p]=static_cast<uchar>(x[p]);

            p++;
        }
    }
//    for(int j=1;j<width+1;j++)
//    {
//        for(int n=1;n<height+1;n++)
//        {
//            resBuffer[p]=static_cast<uchar>(buffer2dimension_large[j][n]*filter[4]+buffer2dimension_large[j][n+1]*filter[5]+buffer2dimension_large[j][n-1]*filter[3]
//                                           +buffer2dimension_large[j+1][n]*filter[7]+buffer2dimension_large[j+1][n+1]*filter[8]+buffer2dimension_large[j+1][n-1]*filter[6]
//                                           +buffer2dimension_large[j-1][n]*filter[1]+buffer2dimension_large[j-1][n+1]*filter[2]+buffer2dimension_large[j-1][n-1]*filter[0]);

//            p++;
//        }
//    }
    return resBuffer;
}
#pragma endregion}

#pragma region"傅里叶变换，自己写的，四嵌套循环，效率低下"{
void operation::fuliye(float *buffer,int width,int height,int bandsize)
{
    QVector<QVector<float>>buffer2dimension;

    // 初始化行和列
    buffer2dimension.resize(width);
    for(int j=0;j<buffer2dimension.size();j++)
    {
        buffer2dimension[j].resize(height);
    }
    // 一维数组转成二维数组
    int m=0;
    for(int i=0;i<width;i++)
    {
        for(int j=0;j<height;j++)
        {
            buffer2dimension[i][j]=buffer[m];
            m++;
        }
    }

    QVector<float> R; // 实部
    QVector<float> I; // 虚部
    float temp_R=0;
    float temp_I=0;
    for(int u=0;u<width;u++)
    {
        for(int v=0;v<height;v++)
        {
            for(int x=0;x<width;x++)
            {
                for(int y=0;y<height;y++)
                {
                    int u=x;
                    int v=y;
                    temp_R+=buffer2dimension[x][y]*qCos(-2*M_PI*(u*x/width+v*y/height));
                    temp_I+=buffer2dimension[x][y]*qSin(-2*M_PI*(u*x/width+v*y/height));
                }
            }
            R.append(temp_R/bandsize);
            I.append(temp_I/bandsize);
        }
    }
    QVector<float> P; // 频率谱，傅里叶变换的振幅，可以用二维图像显示
    for(int i=0;i<R.length();i++)
    {
        P.append(qSqrt(qPow(R[i],2)+qPow(I[i],2)));
    }
}
#pragma endregion}

#pragma region"opencv+dft函数实现傅里叶变换"{
unsigned char* operation::DFT(float *buffer,int row,int col)
{
    // 将float*类型转成Mat类型，方便后续的DFT函数执行
    cv::Mat buffer2dimension(row, col, CV_64F);
    for (int i = 0; i < row; i++)
    {
        for(int j = 0;j < col; j++)
            buffer2dimension.at<double>(i, j) = buffer[i*col + j];
    }

    // 读取灰度图像
    cv::Mat imgGray = buffer2dimension;
    // 获取图像的行(高度)和列(宽度)
    int rows = imgGray.rows;
    int cols = imgGray.cols;

    // 最优DFT扩充尺寸
    int rPad = cv::getOptimalDFTSize(rows);
    int cPad = cv::getOptimalDFTSize(cols);

    // 对原始图像进行边缘扩充
    cv::Mat imgEx = cv::Mat::zeros(rPad, cPad, CV_64F);
    // 手动将 imgGray 复制到 imgEx 中
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            imgEx.at<double>(i, j) = imgGray.at<double>(i, j);
        }
    }

    // 进行快速傅里叶变换
    cv::dft(imgEx, imgEx, cv::DFT_COMPLEX_OUTPUT);

    if(dftnum == 1) // 理想低通滤波
    {
        // 生成低通滤波器
        cv::Mat lowpass(rPad, cPad, CV_64F);

        double cutoff_frequency = 0.7;  // 低通滤波器的截止频率

        // 生成二维低通滤波器
        for (int i = 0; i < rPad; ++i) {
            for (int j = 0; j < cPad; ++j) {
                double distance = std::sqrt((i - rPad / 2.0) * (i - rPad / 2.0) +
                                            (j - cPad / 2.0) * (j - cPad / 2.0));

                // 截止频率以内的部分设为1，截止频率以外的部分设为0
                lowpass.at<double>(i, j) = (distance <= cutoff_frequency * rPad) ? 1.0 : 0.0;
            }
        }

        // 将低通滤波器应用于频域图像
        cv::Mat imgEx_lowpass = imgEx.clone();
        cv::Mat planes1[] = {cv::Mat_<double>(lowpass), cv::Mat::zeros(lowpass.size(), CV_64F)};
        cv::merge(planes1, 2, lowpass);  // 合并实部和虚部

        // 乘以低通滤波器
        cv::mulSpectrums(imgEx, lowpass, imgEx, 0);
    }
    else if(dftnum == 2) // 高斯滤波
    {
        cv::Mat lowpass(rPad, cPad, CV_64F);

        // 计算Gaussian滤波器
        double sigma = 250;  // 高斯函数的标准差

        // 构建二维Gaussian滤波器
        cv::Mat kernel(rPad, cPad,CV_64F);
        for (int i = 0; i < rPad; ++i) {
            for (int j = 0; j < cPad; ++j) {
                double distance = qSqrt((i - rPad / 2.0) * (i - rPad / 2.0) +
                                        (j - cPad / 2.0) * (j - cPad / 2.0)); //计算当前元素到矩阵中心的欧几里得距离。rows / 2.0 和 cols / 2.0 表示图像的中心坐标
                kernel.at<double>(i, j) = qExp(-distance * distance / ( 2 * sigma * sigma));
            }
        }

        cv::Mat imgEx_lowpass = imgEx.clone();
        cv::Mat planes1[] = {cv::Mat_<double>(kernel), cv::Mat::zeros(kernel.size(), CV_64F)};
        cv::merge(planes1, 2, kernel);  // 合并实部和虚部
        // 在频域应用Gaussian低通滤波器
        cv::Mat img_lowpass;
        cv::mulSpectrums(imgEx, kernel, imgEx, 0);
    }
    else if(dftnum == 3) // 高通滤波
    {
        // 创建Butterworth高通滤波器
        cv::Mat butterworthHighPassFilter = cv::Mat::zeros(rPad, cPad, CV_64FC2);

        // 中心坐标
        int centerI = rPad / 2;
        int centerJ = cPad / 2;

        // 阶数（可根据需要调整）
        int order = 1;

        // 截止频率（可根据需要调整）
        double cutoffFrequency = 10000;

        // 设置Butterworth高通滤波器
        double radius;
        for (int i = 0; i < rPad; i++)
        {
            for (int j = 0; j < cPad; j++)
            {
                radius = sqrt(pow(i - centerI, 2) + pow(j - centerJ, 2));

                // 计算Butterworth高通滤波器响应
                double response = 1.0 / (1.0 + pow(cutoffFrequency / radius, 2 * order));

                // 通过频率响应设置Butterworth高通滤波器
                butterworthHighPassFilter.at<cv::Vec2d>(i, j) = cv::Vec2d(response, response);
            }
        }

        // 应用高通滤波器
        cv::Mat filteredImage;
        cv::mulSpectrums(imgEx, butterworthHighPassFilter, imgEx, 0, true);
//        // 使用拉普拉斯算子进行高通滤波
//        cv::Mat img_laplacian;
//        cv::Laplacian(imgEx, imgEx, CV_64F);
    }


    cv::idft(imgEx, imgEx); // 输出图像为复数图像

    // 创建两个单通道的 Mat 对象，用于保存实部和虚部
    Mat realPart(imgEx.size(), CV_64F); // 实部
    Mat imagPart(imgEx.size(), CV_64F); // 虚部
    // 分别提取实部和虚部
    Mat planes[] = {realPart, imagPart};
    split(imgEx, planes);
    // 获取逆傅里叶变换的幅度谱
    cv::Mat magnitude_spectrum;
    cv::magnitude(planes[0],planes[1], magnitude_spectrum);
    cv::Mat idftMagNorm;
    cv::normalize(magnitude_spectrum, idftMagNorm, 0, 255, cv::NORM_MINMAX);

    // 恢复图像（矩阵裁剪）
    cv::Mat imgRebuild = idftMagNorm(cv::Rect(0, 0, cols, rows));

    unsigned char* resBuffer = new unsigned char[rows*cols];
    // 复制数据到 resBuffer
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            resBuffer[i * cols + j] = static_cast<unsigned char>(imgRebuild.at<double>(i,j));
        }
    }
    return resBuffer;
}
#pragma endregion}

#pragma region"影像粗校正"{
void operation::ImageRegistration(float b,float l,float z,float Phi,float Kappa,float Omega,float Camera_f,float Camera_w,float Camera_h,float outSize)
{
    // 影像数据 影像的尺寸例如1024*1024，波段信息例如灰度值这些
    double adfGeoTransform[6];
    rasterDataset->GetGeoTransform(adfGeoTransform);
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();

    QVector<QVector<int>> band;
    QVector<int> rband;
    QVector<int> gband;
    QVector<int> bband;
    // 首先分别读取RGB三个波段
    float * rBand = new float[w * h];
    float * gBand = new float[w * h];
    float * bBand = new float[w * h];

    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, w, h, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, w, h, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, w, h, GDT_Float32, 0, 0);
    // 这里省略了一部分，默认纠正图像为8bit，所以就没有进行拉伸
    for(int i = 0;i < w*h;i++)
        rband.append(rBand[i]);
    for(int i = 0;i < w*h;i++)
        gband.append(gBand[i]);
    for(int i = 0;i < w*h;i++)
        bband.append(bBand[i]);
    band.append(rband);
    band.append(gband);
    band.append(bband);

    // 校正参数，将无人机拍摄瞬间的经纬度转成大地坐标 调用函数toXY
    double Xs = b;
    double Ys = l;
    double Zs = z;
    QVector<double> Xs_Ys = toXY(b,l);
    Xs = Xs_Ys[0];
    Ys = Xs_Ys[1];
    // 无人机成像参数单位换算
    double phi = Phi / 180.0 * M_PI;
    double omega = Omega / 180.0 * M_PI;
    double kappa = Kappa / 180.0 * M_PI;
    double Iw = Camera_w * 0.001;
    double Ih = Camera_h * 0.001;
    double f = Camera_f * 0.001;
    double u = outSize;
    // 计算旋转矩阵
    double a1 = qCos(phi) * qCos(kappa) - qSin(phi) * qSin(omega) * qSin(kappa);
    double a2 = -qCos(phi) * qSin(kappa) - qSin(phi) * qSin(omega) * qCos(kappa);
    double a3 = -qSin(phi) * qCos(omega);
    double b1 = qCos(omega) * qSin(kappa);
    double b2 = qCos(omega) * qCos(kappa);
    double b3 = -qSin(omega);
    double c1 = qSin(phi) * qCos(kappa) + qCos(phi) * qSin(omega) * qSin(kappa);
    double c2 = -qSin(phi) * qSin(kappa) + qCos(phi) * qSin(omega) * qSin(kappa);
    double c3 = qCos(phi) * qCos(kappa);

    // 计算影像四个角点的地理坐标，利用共线方程
    QVector<QVector<float>> Ixy; // 计算像平面坐标
    QVector<float> LT;
    QVector<float> RT;
    QVector<float> LB;
    QVector<float> RB;
    LT.append(0);
    LT.append(Ih);
    RT.append(Iw);
    RT.append(Ih);
    LB.append(0);
    LB.append(0);
    RB.append(Iw);
    RB.append(0);

    Ixy.append(LT);
    Ixy.append(RT);
    Ixy.append(LB);
    Ixy.append(RB);

    // 角点地面坐标
    double Zp = 750.0;
    QVector<QVector<float>> XY; // 角点地面坐标

    for(int i = 0;i < 4;i++)
    {
        QVector<float> temp; // 用于循环
        //        float temp_X = adfGeoTransform[0] + Ixy[i][0] * adfGeoTransform[1] + Ixy[i][1] * adfGeoTransform[2];
        //        float temp_Y = adfGeoTransform[3] + Ixy[i][0] * adfGeoTransform[4] + Ixy[i][1] * adfGeoTransform[5];
        float temp_X = Xs + (Zp - Zs) *(a1 * Ixy[i][0] + a2 * Ixy[i][1] - a3 * f) / (c1 * Ixy[i][0] + c2 * Ixy[i][1] - c3 * f);
        float temp_Y = Ys + (Zp - Zs) *(b1 * Ixy[i][0] + b2 * Ixy[i][1] - b3 * f) / (c1 * Ixy[i][0] + c2 * Ixy[i][1] - c3 * f);
        temp.append(temp_X);
        temp.append(temp_Y);
        XY.append(temp);
    }

    //输出影像最小范围 纠正后图像和原始图像的形状、大小、方向都不一样。所以在纠正过程的实施之前，必须首先确定新图像的大小范围，这里通过四个角点纠正后的坐标来确定
    double Xmin = 9999999;
    double Xmax = 0;
    double Ymin = 9999999;
    double Ymax = 0;

    for (int i = 0; i < 4; i++)
    {
        if (Xmax < XY[i][0]) Xmax = XY[i][0];
        if (Xmin > XY[i][0]) Xmin = XY[i][0];
        if (Ymax < XY[i][1]) Ymax = XY[i][1];
        if (Ymin > XY[i][1]) Ymin = XY[i][1];
    }

    int outwid = (int)((Xmax - Xmin) / u);
    int outhei = (int)((Ymax - Ymin) / u);

    QVector<QVector<int>> fin;
    fin.resize(band.size());
    for(int j=0;j<fin.size();j++)
    {
        fin[j].resize(outwid * outhei);
    }

    // 重采样
    for (int i = 0; i < outwid; i++)
    {
        for (int j = 0; j < outhei; j++)
        {
            double Xp = i * u + Xmin;
            double Yp = (outhei - j - 1) * u + Ymin;
            double x = -f * (a1 * (Xp - Xs) + b1 * (Yp - Ys) + c1 * (Zp - Zs)) /
                       (a3 * (Xp - Xs) + b3 * (Yp - Ys) + c3 * (Zp - Zs));
            double y = -f * (a2 * (Xp - Xs) + b2 * (Yp - Ys) + c2 * (Zp - Zs)) /
                       (a3 * (Xp - Xs) + b3 * (Yp - Ys) + c3 * (Zp - Zs));
            // 确定对应原影像像素的行和列
            int i0 = (int)(x * w / Iw);
            int j0 = (int)(y * h / Ih);
            if (i0 >= 0 && i0 < w && j0 >= 0 && j0 < h)
            {
                for (int k = 0; k < fin.size(); k++)
                {
                    fin[k][i + j * outwid] = band[k][i0 + j0 * w];
                }
            }
            else
            {
                for (int k = 0; k < fin.size(); k++)
                {
                    fin[k][i + j * outwid] = 0;
                }
            }
        }
    }
    // 将三个波段组合起来 用于显示影像
    int bytePerLine = (outwid * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * outhei];
    for (int h = 0; h < outhei; h++)
    {
        for (int w = 0; w < outwid; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = fin[0][h * outwid + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = fin[1][h * outwid + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = fin[2][h * outwid + w];
        }
    }

    image=new QImage(allBandUC, outwid, outhei, bytePerLine, QImage::Format_RGB888);
    image->save("x.jpg");
    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}

QVector<double> operation::toXY(double B,double L)
{
    double l = calL(L); // 计算中央子午线
    double b = B;
    double X = calX(b); // 计算大地线长
    double t = qTan(r(b));
    double n = qSqrt(ee * ee * qCos(r(b)) * qCos(r(b)));
    double x1 = caln(b) * qSin(r(b)) * qCos(r(b)) * l * l / (2.0 * p * p);
    double x2 = caln(b) * qSin(r(b)) * qCos(r(b)) * qCos(r(b)) * qCos(r(b)) * (5.0 - t * t + 9 * n * n + 4 * n * n * n * n) * l * l * l * l / (24 * p * p * p * p);
    double x3 = caln(b) * qSin(r(b)) * qCos(r(b)) * qCos(r(b)) * qCos(r(b)) * qCos(r(b)) * qCos(r(b)) * (61 - 58 * t * t + t * t * t * t) * l * l * l * l * l * l / (720 * p * p * p * p * p * p);

    double y1 = caln(b) * qCos(r(b)) * l / p;
    double y2 = caln(b) * qCos(r(b)) * qCos(r(b)) * qCos(r(b)) * (1.0 - t * t + n * n) * l * l * l / (6.0 * p * p * p);
    double y3 = caln(b) * qCos(r(b)) * qCos(r(b)) * qCos(r(b)) * qCos(r(b)) * qCos(r(b)) * (5.0 - 18.0 * t * t + t * t * t * t + 14.0 * n * n - 58.0 * n * n * t * t) * l * l * l * l * l / (120.0 * p * p * p * p * p);

    QVector<double> xy;
    double x = X + x1 + x2 + x3;
    double y = y1 + y2 + y3;
    xy.append(x);
    xy.append(y);

    return xy;
}

// 度转弧度
double operation::r(double x)
{
    double y = x * M_PI / 180.0;
    return y;
}

double operation::calm(double x)
{
    x = r(x);
    double y = qSqrt(1 - e * e * qSin(x) * qSin(x));
    double z = a * (1 - e * e) / (y * y * y);
    return z;
}

double operation::caln(double x)
{
    x = r(x);
    double y = qSqrt(1 - e * e * qSin(x) * qSin(x));
    double z = a / y;
    return z;
}

double operation::calX(double b)
{
    double y = a0 * r(b) - a2 * qSin(2 * r(b)) / 2 + a4 * qSin(4 * r(b)) / 4 - a6 * qSin(6 * r(b)) / 6 + a8 * qSin(8 * r(b)) / 8;
    return y;
}

double operation::calL(double L)
{
    int n = 0;
    double l1 = L;
    l1 = l1 / 6;
    int l2 = (int)l1;
    if (l1 != l2)
    {
        n = (int)L / 6 + 1;
    }
    else
    {
        n = (int)L / 6;
    }
    double L0 = 6 * n - 3;
    double l = 0;
    if (L - L0 > 0)
    {
        l = L - L0;
    }
    else
    {
        l = L - L0 + 6;
    }
    l = l * 3600;
    return l;
}
#pragma endregion}

#pragma region"手动配准+多项式配准模型+仿射变换公式（还未用到）"{
QVector<float> operation::duoxiangshi(QVector<QPointF> A_array,QVector<QPointF> L_array)
{
    QVector<float> result;

    MatrixXf A(3,3);
    A << 1,L_array[0].x(), L_array[0].y(),
         1,L_array[1].x(), L_array[1].y(),
         1,L_array[2].x(), L_array[2].y();
    MatrixXf b_x(3,1);
    b_x << A_array[0].x(),
           A_array[1].x(),
           A_array[2].x();
    MatrixXf b_y(3,1);
    b_y << A_array[0].y(),
           A_array[1].y(),
           A_array[2].y();
//    MatrixXf x(3,1);
//    x=(A.transpose()*A).inverse()*A.transpose()*b_x;
//    MatrixXf x1(3,1);
//    x1=(A.transpose()*A).inverse()*A.transpose()*b_y;

    // 使用QR分解求解
    MatrixXf x = A.householderQr().solve(b_x);
    MatrixXf x1 = A.householderQr().solve(b_y);

    for(int i=0;i<x.rows();i++)
    {
        result.append(x(i,0));
    }
    for(int i=0;i<x1.rows();i++)
    {
        result.append(x1(i,0));
    }
    return result;
}

void operation::handpeizhun(QVector<int> src_x,QVector<int>src_y,QVector<int>ref_x,QVector<int>ref_y,QVector<int> psrc_x,QVector<int>psrc_y,QVector<int>pref_x,QVector<int>pref_y,QVector<double> geo)
{
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    QVector<QVector<int>> band;
    QVector<int> rband;
    QVector<int> gband;
    QVector<int> bband;
    // 首先分别读取RGB三个波段
    float * rBand = new float[w*h];
    float * gBand = new float[w*h];
    float * bBand = new float[w*h];

    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, w, h, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, w, h, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, w, h, GDT_Float32, 0, 0);

    // 把三个通道的灰度值缩放到0-255之间
    rBand =Stretch255(rBand,rasterDataset->GetRasterBand(1),w*h);
    gBand =Stretch255(gBand,rasterDataset->GetRasterBand(2),w*h);
    bBand =Stretch255(bBand,rasterDataset->GetRasterBand(3),w*h);
    // 这里省略了一部分，默认纠正图像为8bit，所以就没有进行拉伸
    for(int i = 0;i < w*h;i++)
        rband.append(rBand[i]);
    for(int i = 0;i < w*h;i++)
        gband.append(gBand[i]);
    for(int i = 0;i < w*h;i++)
        bband.append(bBand[i]);
    band.append(rband);
    band.append(gband);
    band.append(bband);

    // 待配准影像and标准影像的三对控制点的地理坐标
    QVector<QPointF> pre;
    QVector<QPointF> ref;
    QPointF pre1(src_x[0],src_y[0]);
    QPointF pre2(src_x[1],src_y[1]);
    QPointF pre3(src_x[2],src_y[2]);
    QPointF pre4(src_x[3],src_y[3]);
    QPointF ori1(ref_x[0],ref_y[0]);
    QPointF ori2(ref_x[1],ref_y[1]);
    QPointF ori3(ref_x[2],ref_y[2]);
    QPointF ori4(ref_x[3],ref_y[3]);
//    pre.append(pre1);
//    pre.append(pre2);
//    pre.append(pre3);
//    pre.append(pre4);
//    ref.append(ori1);
//    ref.append(ori2);
//    ref.append(ori3);
//    ref.append(ori4);
//    QPointF pre1(402346.320212,3403947.758393);
//    QPointF pre2(403118.799257,3404346.010023);
//    QPointF pre3(402367.033809,3404603.321755);
//    QPointF pre4(401908.268248,3404616.007088);
//    QPointF ori1(402349.848306,3403947.701178);
//    QPointF ori2(403125.714024,3404351.032818);
//    QPointF ori3(402366.760506,3404609.055001);
//    QPointF ori4(401902.786245,3404619.215021);
    pre.append(pre1);
    pre.append(pre2);
    pre.append(pre3);
    pre.append(pre4);
    ref.append(ori1);
    ref.append(ori2);
    ref.append(ori3);
    ref.append(ori4);

    QVector<float> para = duoxiangshi(pre,ref); // 标准到待配准的六参数
    QVector<float> para1 = duoxiangshi(ref,pre); // 待配准到标准的六参数

    // 计算影像四个角点的地理坐标
    QVector<QVector<float>> Ixy; // 计算像平面坐标
    QVector<float> LT;
    QVector<float> RT;
    QVector<float> LB;
    QVector<float> RB;
    LT.append(0);
    LT.append(0);
    RT.append(w);
    RT.append(0);
    LB.append(0);
    LB.append(h);
    RB.append(w);
    RB.append(h);

    Ixy.append(LT);
    Ixy.append(RT);
    Ixy.append(LB);
    Ixy.append(RB);

    QVector<QVector<float>> XY; // 角点地面坐标

    for(int i = 0;i < 4;i++)
    {
        QVector<float> temp; // 用于循环
        float temp_X = geo[0] + Ixy[i][0] * geo[1] + Ixy[i][1] * geo[2];
        float temp_Y = geo[3] + Ixy[i][0] * geo[4] + Ixy[i][1] * geo[5];
        temp_X = para1[0] + temp_X*para1[1] + temp_Y*para1[2];
        temp_Y = para1[3] + temp_X*para1[4] + temp_Y*para1[5];
        temp.append(temp_X);
        temp.append(temp_Y);
        XY.append(temp);
    }

    //输出影像最小范围 纠正后图像和原始图像的形状、大小、方向都不一样。所以在纠正过程的实施之前，必须首先确定新图像的大小范围，这里通过四个角点纠正后的坐标来确定
    double Xmin = 9999999;
    double Xmax = 0;
    double Ymin = 9999999;
    double Ymax = 0;

    for (int i = 0; i < 4; i++)
    {
        if (Xmax < XY[i][0]) Xmax = XY[i][0];
        if (Xmin > XY[i][0]) Xmin = XY[i][0];
        if (Ymax < XY[i][1]) Ymax = XY[i][1];
        if (Ymin > XY[i][1]) Ymin = XY[i][1];
    }

    int outwid = (int)((Xmax - Xmin) / 5);
    int outhei = (int)((Ymax - Ymin) / 5);

    QVector<QVector<int>> fin;
    fin.resize(band.size());
    for(int j=0;j<fin.size();j++)
    {
        fin[j].resize(outwid * outhei);
    }

    // 重采样
    for (int i = 0; i < outwid; i++)
    {
        for (int j = 0; j < outhei; j++)
        {
            double Xp = i * 5 + Xmin;
            double Yp = (outhei - j - 1) * 5 + Ymin; // 纠正后的地理坐标

            double x = para[0] + Xp * para[1] + Yp * para[2];
            double y = para[3] + Xp * para[4] + Yp * para[5]; // 纠正前的地理坐标，即原影像
            // 确定对应原影像像素的行和列
            double dTemp = geo[1] * geo[5] - geo[2] *geo[4];
            int i0= (geo[5] * (x - geo[0]) -geo[2] * (y - geo[3])) / dTemp + 0.5;
            int j0 = (geo[1] * (y - geo[3]) -geo[4] * (x - geo[0])) / dTemp + 0.5;

            if (i0 >= 0 && i0 < w && j0 >= 0 && j0 < h)
            {
                for (int k = 0; k < fin.size(); k++)
                {
                    fin[k][i + j * outwid] = band[k][i0 + j0 * w];
                }
            }
            else
            {
                for (int k = 0; k < fin.size(); k++)
                {
                    fin[k][i + j * outwid] = 0;
                }
            }
        }
    }

    // 将三个波段组合起来 用于显示影像
    int bytePerLine = (outwid * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * outhei];
    for (int h = 0; h < outhei; h++)
    {
        for (int w = 0; w < outwid; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = fin[0][h * outwid + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = fin[1][h * outwid + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = fin[2][h * outwid + w];
        }
    }

    image=new QImage(allBandUC, outwid, outhei, bytePerLine, QImage::Format_RGB888);
    image->save("x.jpg");
    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma region"opencv+FLANN+SIFT自动配准"{
QGraphicsScene * operation::SIFT()
{
    QString fileName = QFileDialog::getOpenFileName(0,QFileDialog::tr("Open Image"),".",QFileDialog::tr("Image File(*.png *.jpg *.jpeg *.tif *.img)"));
    QFile file(fileName);

    QGraphicsScene* mysceneSIFT = new QGraphicsScene();

    if(n==1)
    {
        if(!file.open(QFile::ReadOnly))
        {
            qDebug("读取失败");
        }
        else{
            QByteArray ba = file.readAll();
            image1 = imdecode(std::vector<char>(ba.begin(), ba.end()), 1);
        }
        if(image1.empty ()){
            qDebug("图像为空");
        }
        else
        {
            cvtColor(image1,image1,COLOR_BGR2RGB);
            // 找出特征点 特征点检测
            cv::Ptr<cv::SIFT> featureDetector = cv::SIFT::create();
            featureDetector->detect(image1,image1_kp);
            // 画出特征点
            drawKeypoints(image1,image1_kp,image1,Scalar(255,0,0),DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
            // 显示在qimage控件中 Mat转Qimage
            QImage *img1 =new QImage((const unsigned char*)(image1.data),image1.cols,image1.rows,image1.step,QImage::Format_RGB888);
            QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*img1));
            mysceneSIFT->addItem(imgItem);
        }
    }
    else
    {
        if(!file.open(QFile::ReadOnly))
        {
            qDebug("读取失败");
        }
        else{
            QByteArray ba = file.readAll();
            image2 = imdecode(std::vector<char>(ba.begin(), ba.end()), 1);
        }
        if(image2.empty ()){
            qDebug("图像为空");
        }
        else
        {
            cvtColor(image2,image2,COLOR_BGR2RGB);
            // 找出特征点 特征点检测
            cv::Ptr<cv::SIFT> featureDetector = cv::SIFT::create();
            featureDetector->detect(image2,image2_kp);
            // 画出特征点
            drawKeypoints(image2,image2_kp,image2,Scalar(255,0,0),DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
            // 显示在qimage控件中 Mat转Qimage
            QImage *img2 =new QImage((const unsigned char*)(image2.data),image2.cols,image2.rows,image2.step,QImage::Format_RGB888);
            QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*img2));
            mysceneSIFT->addItem(imgItem);
        }
    }
    return mysceneSIFT;
}

QGraphicsScene *operation::SIFT_FLANN()
{
    QGraphicsScene* mysceneSIFT = new QGraphicsScene();
    Mat image1_desc,image2_desc; //特征点描述符
    //特征点提取
    cv::Ptr<cv::SIFT> featureExtractor = cv::SIFT::create();
    featureExtractor->compute(image1,image1_kp, image1_desc);
    featureExtractor->compute(image2,image2_kp, image2_desc);

    //FLANN based descriptor matcher object
    // FLANN算法
    FlannBasedMatcher matcher;
    std::vector<Mat> image1_desc_collection(1, image1_desc);
    matcher.add(image1_desc_collection);
    matcher.train();

    //match image1 and image2 descriptor,getting 200 nearest neighbors for all test descriptor
    // 找到所有的匹配点，但是有正确的也有错误的，后续进行筛选
    std::vector<std::vector<DMatch> > matches;
    matcher.knnMatch(image2_desc, matches, 2);

    //filter for good matches according to Lowe's algorithm
    // 过滤，筛选出错误的匹配特征点
    std::vector<DMatch> good_matches;
    for (unsigned int i = 0; i < 0.6*matches.size(); i++)
    {
        if (matches[i][0].distance < matches[i][1].distance)
            good_matches.push_back(matches[i][0]);
    }

    // 显示匹配好的图像
    Mat img_show;
    drawMatches(image2, image2_kp, image1, image1_kp, good_matches, img_show,Scalar(255, 0, 0));
    QImage* img3 =new QImage((const unsigned char*)(img_show.data),img_show.cols,img_show.rows,img_show.step,QImage::Format_RGB888);
    img3->save("F:\\111.jpg");
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*img3));
    mysceneSIFT->addItem(imgItem);

    return mysceneSIFT;
}
#pragma endregion}

#pragma region"影像融合"{

#pragma region"影像融合之Brovey"{
void operation::Brovey()
{
    // 高分辨率影像 4096*4096
    int high_w = highRaster -> GetRasterXSize();
    int high_h = highRaster -> GetRasterYSize();
    // 首先分别读取RGB三个波段
    float * rBand1 = new float[high_w * high_h];

    highRaster->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, high_w, high_h, rBand1, high_w, high_h, GDT_Float32, 0, 0);

    // 低分辨率影像重采样 1024*1024变成4096*4096
    int low_w = lowRaster -> GetRasterXSize();
    int low_h = lowRaster -> GetRasterYSize();
    // 首先分别读取RGB三个波段
    float * rBand = new float[high_w * high_h];
    float * gBand = new float[high_w * high_h];
    float * bBand = new float[high_w * high_h];

    lowRaster->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, low_w, low_h, rBand, high_w, high_h, GDT_Float32, 0, 0);
    lowRaster->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, low_w, low_h, gBand, high_w, high_h, GDT_Float32, 0, 0);
    lowRaster->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, low_w, low_h, bBand, high_w, high_h, GDT_Float32, 0, 0);

    rBand1 = Stretch255(rBand1,highRaster->GetRasterBand(1),high_w * high_h);
    rBand = Stretch255(rBand,lowRaster->GetRasterBand(1),high_w * high_h);
    gBand = Stretch255(gBand,lowRaster->GetRasterBand(2),high_w * high_h);
    bBand = Stretch255(bBand,lowRaster->GetRasterBand(3),high_w * high_h);

//    QVector<float> r;
//    QVector<float> g;
//    QVector<float> b;
    float * r = new float[high_w * high_h];
    float * g = new float[high_w * high_h];
    float * b = new float[high_w * high_h];
    // 逐像元融合
    for(int i = 0;i < high_w * high_h;i++)
    {
        r[i] = (rBand[i] * rBand1[i] / (rBand[i] + gBand[i] + bBand[i]));
        g[i] = (gBand[i] * rBand1[i] / (rBand[i] + gBand[i] + bBand[i]));
        b[i] = (bBand[i] * rBand1[i] / (rBand[i] + gBand[i] + bBand[i]));
    }

//    QVector<float> Get_R_HisMaxMin;
//    QVector<float> Get_G_HisMaxMin;
//    QVector<float> Get_B_HisMaxMin;

//    // 最大值、最小值
//    Get_R_HisMaxMin=GetHistormMAXMin(r,high_w * high_h);
//    Get_G_HisMaxMin=GetHistormMAXMin(g,high_w * high_h);
//    Get_B_HisMaxMin=GetHistormMAXMin(b,high_w * high_h);

//    for(int i = 0;i < high_w * high_h;i++)
//    {
//        r[i] =  255 - 255 * (Get_R_HisMaxMin[0] -r[i]) / (Get_R_HisMaxMin[0] - Get_R_HisMaxMin[1]);
//        g[i] =  255 - 255 * (Get_G_HisMaxMin[0] -g[i]) / (Get_G_HisMaxMin[0] - Get_G_HisMaxMin[1]);
//        b[i] =  255 - 255 * (Get_G_HisMaxMin[0] -b[i]) / (Get_B_HisMaxMin[0] - Get_B_HisMaxMin[1]);
//    }

    // 将三个波段组合起来
    int bytePerLine = (high_w * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * high_h];
    for (int h = 0; h < high_h; h++)
    {
        for (int w = 0; w < high_w; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = r[h * high_w + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = g[h * high_w + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = b[h * high_w + w];
        }
    }

    image=new QImage(allBandUC, high_w, high_h, bytePerLine, QImage::Format_RGB888);
    image->save("Brovey.jpg");

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma region"影像融合之IHS"{
void operation::IHS()
{
    // 高分辨率影像 4096*4096
    int high_w = lowRaster -> GetRasterXSize();
    int high_h = lowRaster -> GetRasterYSize();
    // 首先分别读取RGB三个波段
    float * hBand = new float[high_w * high_h];

    highRaster->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, high_w, high_h, hBand, high_w, high_h, GDT_Float32, 0, 0);

    // 低分辨率影像重采样 1024*1024变成4096*4096 rastrtio函数自动进行了重采样工作
    int low_w = lowRaster -> GetRasterXSize();
    int low_h = lowRaster -> GetRasterYSize();
    // 首先分别读取RGB三个波段
    float * rBand = new float[high_w * high_h];
    float * gBand = new float[high_w * high_h];
    float * bBand = new float[high_w * high_h];

    lowRaster->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, low_w, low_h, rBand, high_w, high_h, GDT_Float32, 0, 0);
    lowRaster->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, low_w, low_h, gBand, high_w, high_h, GDT_Float32, 0, 0);
    lowRaster->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, low_w, low_h, bBand, high_w, high_h, GDT_Float32, 0, 0);

    // 线性拉伸0-255
    hBand = Stretch255(hBand,highRaster->GetRasterBand(1),high_w * high_h);
    rBand = Stretch255(rBand,lowRaster->GetRasterBand(1),high_w * high_h);
    gBand = Stretch255(gBand,lowRaster->GetRasterBand(2),high_w * high_h);
    bBand = Stretch255(bBand,lowRaster->GetRasterBand(3),high_w * high_h);

    // 将多光谱影像变换到IHS空间（RGB 空间变换到IHS空间）
    QVector<float> II;
    QVector<float> H;
    QVector<float> S;
    for(int i = 0;i < high_w * high_h;i++)
    {
        // I0
        float I0 = rBand[i] + gBand[i] + bBand[i];
        // H0
        float min = rBand[i];
        float H0 = 0;
        float S0 = 0;
        if(min > gBand[i])
        {
            min = gBand[i];
            if (min > bBand[i])
            {
                min = bBand[i];
            }
        }
        else
        {
            if(min > bBand[i])
            {
                min = bBand[i];
            }
        }
        if(min == bBand[i])
        {
            H0 = (gBand[i] - bBand[i]) / (I0 - 3*bBand[i]);
        }
        else if(min == rBand[i])
        {
            H0 = 1 + (bBand[i] - rBand[i]) / (I0 - 3*rBand[i]);
        }
        else
        {
            H0 = 2 + (rBand[i] - gBand[i]) / (I0 - 3*gBand[i]);
        }
        // S0
        if (H0 >= 0 && H0 <= 1 )
        {
            S0 = (I0 - 3*bBand[i]) / I0;
        }
        else if(H0 >= 1  && H0 <= 2)
        {
            S0 = (I0 - 3*rBand[i]) / I0;
        }
        else if(H0 >= 2 && H0 <= 3)
        {
            S0 = (I0 - 3*gBand[i]) / I0;
        }

        // 添加到数组里面
        II.append(I0);
        H.append(H0);
        S.append(S0);
    }
    float *I = new float[II.length()];
    for(int i=0;i<II.length();i++)
    {
        I[i] = II[i];
    }
    // 对全色影像和IHS空间中的亮度分量I进行直方图匹配，得到新的全色影像I’
    // 参考图像：高分辨率影像
    // 原图像：影像的I空间
    QVector<QVector<float>> highHistogram = HistogramMatching(hBand,high_h * high_w); // 高分辨率影像累计直方图
    QVector<QVector<float>> lowHistogramI = HistogramMatching(I,high_h * high_w); // IHS空间中I的累计直方图

    // 对II进行扩展，添加上对应的累计频率
    QVector<QVector<float>> I1;
    I1.resize(lowHistogramI.length());
    for(int i = 0;i < high_h * high_w;i++)
    {
        float temp = II[i];
        for(int j =0;j < lowHistogramI[0].length();j++)
        {
            float m = lowHistogramI[0][j];
            if (temp == m)
            {
                I1[0].append(temp);
                I1[1].append(lowHistogramI[1][j]);
            }
            else
            {
                continue;
            }
        }
    }

    // 对上述得到的累计直方图进行匹配 用全色影像I’代替IHS空间的亮度分量，即得到I’HS。
    for(int i = 0;i < highHistogram.length();i++)
    {
        for(int j = 0;j< I1[0].length();j++)
        {
            if(abs(highHistogram[1][i] - I1[1][j]) < 0.001)
            {
                I1[0][j] = highHistogram[0][i];
            }
        }
    }

    // 将I’HS逆变换到RGB空间，即得到融合影像。
    // 输入：lowHistogramI 、H 、S
    // 输出：newR 、 newG 、newb
    QVector<float> newR;
    QVector<float> newG;
    QVector<float> newB;
    for(int i = 0;i < high_h * high_w;i++)
    {
        int R = 0;
        int G = 0;
        int B = 0;
        if (H[i] >=0 && H[i] <= 1)
        {
            R = I1[0][i] * (1 + 2*S[i] -3*S[i]*H[i]) / 3;
            G = I1[0][i] * (1 - S[i] + 3*S[i]*H[i]) / 3;
            B = I1[0][i] * (1 - S[i]) / 3;
        }
        else if(H[i] >= 1 && H[i] <= 2)
        {
            R = I1[0][i] * (1 - S[i]) / 3;
            G = I1[0][i] * (1 + 2*S[i] - 3*S[i]*(H[i]-1)) / 3;
            B = I1[0][i] * (1 - S[i] + 3*S[i]*(H[i]-1)) / 3;
        }
        else if(H[i] >= 2 && H[i] <= 3)
        {
            R = I1[0][i] * (1 - S[i] + 3*S[i]*(H[i]-2)) / 3;
            G = I1[0][i] * (1 - S[i]) / 3;
            B = I1[0][i] * (1 + 2*S[i] - 3*S[i]*(H[i]-2)) / 3;
        }
        newR.append(R);
        newG.append(G);
        newB.append(B);
    }
    float * rrr = new float[high_h * high_w];
    float * ggg = new float[high_h * high_w];
    float * bbb = new float[high_h * high_w];
    for(int i = 0;i < newR.length();i++)
    {
        rrr[i] = newR[i];
        ggg[i] = newG[i];
        bbb[i] = newB[i];
    }
    float HH = -1 * QuantitativeEvaluation(rrr,ggg,bbb,high_h,high_w);

    // 将三个波段组合起来
    int bytePerLine = (high_w * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * high_h];
    for (int h = 0; h < high_h; h++)
    {
        for (int w = 0; w < high_w; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = newR[h * high_w + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = newG[h * high_w + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = newB[h * high_w + w];
        }
    }

    image=new QImage(allBandUC, high_w, high_h, bytePerLine, QImage::Format_RGB888);
    image->save("IHS.jpg");

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma region"直方图匹配"{
QVector<QVector<float>> operation::HistogramMatching(float *buffer,int bandSize)
{
    QVector<QVector<float>> band; // 每个灰度值及其对应的频数，升序排序
    QVector<float> band_probability; // 每个灰度值出现的概率
    QVector<float> band_Cumu_probability; // 归一概率
    QVector<QVector<float>> resband; // 拉伸后的灰度值及其对应的累积频率

    band=PaiXu(SataticsBandsValue(buffer,bandSize));
    float sum_pro=0;

    // 计算归一概率 这里计算的是图像有存在的灰度值
    for(int i=0;i<band[0].length();i++)
    {
        float pro=band[1][i]/bandSize;
        sum_pro+=pro;
        band_probability.append(pro);
        band_Cumu_probability.append(sum_pro);
    }

    resband.append(band[0]);
    resband.append(band_Cumu_probability);

    return resband;
}
#pragma endregion}

#pragma region"影像融合质量评价"{
float operation::QuantitativeEvaluation(float *bufferR,float *bufferG,float *bufferB,int w,int h)
{
    float H = 0;
    QVector<QVector<float>> band; // 每个灰度值及其对应的频数，升序排序
    QVector<float> band_probability; // 每个灰度值出现的概率

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[h * w];
    for(int i = 0;i < h * w;i++)
    {
        buffer[i] = (int)(bufferR[i] * 0.114 + bufferG[i] * 0.587 + bufferB[i] * 0.299);
    }

    // 基于信息量的评价
    band=PaiXu(SataticsBandsValue(buffer,h * w));// （灰度值+概数）
    // 计算图像像元灰度值为i的概率
    for(int i = 0;i < band[0].length();i++)
    {
        float pro=band[1][i]/(h * w);
        band_probability.append(pro);
    }
//    QVector<QVector<float>> HH;
//    HH.resize(2);
//    for(int i = 0;i < bandSize;i++)
//    {
//        float temp = buffer[i];
//        for(int j =0;j < band_probability.length();j++)
//        {
//            float m = band[0][j];
//            if (temp == m)
//            {
//                HH[0].append(temp);
//                HH[1].append(band_probability[j]);
//            }
//            else
//            {
//                continue;
//            }
//        }
//    }
    // 计算H值
    for(int i = 0;i < band_probability.length();i++)
    {
        H += band_probability[i] * qLn(band_probability[i]) / qLn(2);
    }

    // 基于清晰度的评价
//    float *buffer1 = new float[h * w];
//    QVector<QVector<float>> bu;
//    bu.resize(w);

//    for(int i = 0;i < w;i++)
//    {
//        for(int j = 0;j < h;j++)
//        {
//            buffer1[i * w +j] = buffer[i];
//        }
//    }
    float g = 0;
    for(int i = 1;i < w-1;i++)
    {
        for(int j = 1;j < h-1;j++)
        {
            g += qSqrt((qPow(buffer[i * w + j] - buffer[(i + 1) * w + j],2) + qPow(buffer[i * w + h] - buffer[i * w + (j + 1)],2)) / 2);
        }
    }
    g = g / ((w - 1) * (h - 1));
    return H;
}
#pragma endregion}

#pragma region"影像融合之PCA"{
void operation::PCA()
{
    // 高分辨率影像 4096*4096
    int high_w = lowRaster -> GetRasterXSize();
    int high_h = lowRaster -> GetRasterYSize();
    // 首先分别读取RGB三个波段
    float * hBand = new float[high_w * high_h];

    highRaster->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, high_w, high_h, hBand, high_w, high_h, GDT_Float32, 0, 0);

    // 低分辨率影像重采样 1024*1024变成4096*4096 rastrtio函数自动进行了重采样工作
    int low_w = lowRaster -> GetRasterXSize();
    int low_h = lowRaster -> GetRasterYSize();
    // 首先分别读取RGB三个波段
    float * rBand = new float[high_w * high_h];
    float * gBand = new float[high_w * high_h];
    float * bBand = new float[high_w * high_h];

    lowRaster->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, low_w, low_h, rBand, high_w, high_h, GDT_Float32, 0, 0);
    lowRaster->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, low_w, low_h, gBand, high_w, high_h, GDT_Float32, 0, 0);
    lowRaster->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, low_w, low_h, bBand, high_w, high_h, GDT_Float32, 0, 0);

    // 线性拉伸0-255
    hBand = Stretch255(hBand,highRaster->GetRasterBand(1),high_w * high_h);
    rBand = Stretch255(rBand,lowRaster->GetRasterBand(1),high_w * high_h);
    gBand = Stretch255(gBand,lowRaster->GetRasterBand(2),high_w * high_h);
    bBand = Stretch255(bBand,lowRaster->GetRasterBand(3),high_w * high_h);

    // 多光谱影像三个通道组成二维向量 尺寸：像素总数*3
    QVector<QVector<float>> X;
    X.resize(3);
    for(int i = 0;i < X.length();i++)
    {
        X[i].resize(high_h * high_w);
    }
    // 三个通道的数据集
    for(int i = 0;i < X.length();i++)
    {
        int m=0;
        for(int j = 0;j < high_h * high_w;j++)
        {
            switch (i) {
            case 0:
                X[i][j] = rBand[m];
                m++;
                break;
            case 1:
                X[i][j] = gBand[m];
                m++;
                break;
            case 2:
                X[i][j] = bBand[m];
                m++;
                break;
            default:
                break;
            }

        }
    }
    // 计算三个通道各个的均值向量
    QVector<float> average;
    for(int i = 0;i < X.length();i++)
    {
        float temp_ave = 0;
        for(int j = 0;j < X[i].length();j++)
        {
            temp_ave += X[i][j];
        }
        average.append(temp_ave / X[i].length());
    }
    // 计算协方差
    QVector<float> variance; // 方差
    QVector<float> covariance; // 协方差
    for(int i = 0;i < X.length();i++)
    {
        float temp_variance = 0;
        for(int j = 0;j < X[i].length();j++)
        {
            temp_variance += qPow(X[i][j] - average[i],2);
        }
        variance.append(temp_variance / (X[i].length() - 1));
    }
    // R 和 G的协方差
    float temp_covarianceRG = 0;
    for(int i = 0;i < X[0].length();i++)
    {
        temp_covarianceRG += (X[0][i] - average[0]) * (X[1][i] - average[1]);
    }
    // R 和 B的协方差
    float temp_covarianceRB = 0;
    for(int i = 0;i < X[1].length();i++)
    {
        temp_covarianceRB += (X[0][i] - average[0]) * (X[2][i] - average[2]);
    }
    // G 和 B的协方差
    float temp_covarianceGB = 0;
    for(int i = 0;i < X[2].length();i++)
    {
        temp_covarianceGB += (X[1][i] - average[1]) * (X[2][i] - average[2]);
    }
    covariance.append(temp_covarianceRG / (X[0].length() - 1));
    covariance.append(temp_covarianceRB / (X[1].length() - 1));
    covariance.append(temp_covarianceGB / (X[2].length() - 1));
    // 求协方差矩阵的特征值和特征向量 用Eigen库
    MatrixXf m(3, 3);
    m << variance[0],covariance[0],covariance[1],
         covariance[0],variance[1],covariance[2],
         covariance[1],covariance[2],variance[2];
    EigenSolver<MatrixXf> es(m);
    MatrixXf value= es.eigenvalues().real(); // 特征值
    MatrixXf vector= es.eigenvectors().real(); // 特征向量
    // 将特征值按由大到小的次序排列（排序）
    std::vector<int> index(value.rows()); // 创建一个索引数组，用于按特征值大小排序
    std::iota(index.begin(), index.end(), 0);  // 填充索引数组
    auto compare = [&value](int i, int j) // 使用 lambda 表达式定义比较函数，用于排序
    {
        return value(i, 0) > value(j, 0);
    };
    std::sort(index.begin(), index.end(), compare); // 对索引数组进行排序
    // 选择前n个特征值对应的n个特征向量构造变换矩阵φn
    Eigen::MatrixXf Phi_n = vector.leftCols(3); // 构造变换矩阵 Phi_n
    // 将 QVector 转换为 Eigen 矩阵
    Eigen::MatrixXf X_eigen(high_h * high_w, 3);
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < high_h * high_w; ++j)
        {
            X_eigen(j, i) = X[i][j];
        }
    }
    // 进行特征变换
    Eigen::MatrixXf Y = Phi_n * X_eigen.transpose();  // 注意这里使用转置
    // 直方图匹配 用全色影像和Y的第一主分量
    float *I = new float[Y.row(0).size()];
    for(int i = 0;i < Y.row(0).size();i++)
    {
        I[i] = int(Y(0,i));
    }
    // 对全色影像和IHS空间中的亮度分量I进行直方图匹配，得到新的全色影像I’
    // 参考图像：高分辨率影像
    // 原图像：影像的I空间
    QVector<QVector<float>> highHistogram = HistogramMatching(hBand,high_h * high_w); // 高分辨率影像累计直方图
    QVector<QVector<float>> lowHistogramI = HistogramMatching(I,high_h * high_w); // IHS空间中I的累计直方图

    // 对II进行扩展，添加上对应的累计频率
    QVector<QVector<float>> I1;
    I1.resize(lowHistogramI.length());
    for(int i = 0;i < high_h * high_w;i++)
    {
        float temp = I[i];
        for(int j =0;j < lowHistogramI[0].length();j++)
        {
            float m = lowHistogramI[0][j];
            if (temp == m)
            {
                I1[0].append(temp);
                I1[1].append(lowHistogramI[1][j]);
            }
            else
            {
                continue;
            }
        }
    }

    // 对上述得到的累计直方图进行匹配 用全色影像I’代替IHS空间的亮度分量，即得到I’HS。
    for(int i = 0;i < highHistogram.length();i++)
    {
        for(int j = 0;j< I1[0].length();j++)
        {
            if(abs(highHistogram[1][i] - I1[1][j]) < 0.001)
            {
                I1[0][j] = highHistogram[0][i];
            }
        }
    }
    // 逆主分量变换
    for(int i = 0;i < I1[0].length();i++)
    {
        Y(0,i) = I1[0][i];
    }
    Y = Phi_n.inverse() * Y;  // 注意这里使用转置
    // 输出影像
    QVector<float> newR;
    QVector<float> newG;
    QVector<float> newB;
    for(int i = 0;i < Y.row(0).size();i++)
    {
        newR.append(Y(0,i));
    }
    for(int i = 0;i < Y.row(1).size();i++)
    {
        newG.append(Y(1,i));
    }
    for(int i = 0;i < Y.row(2).size();i++)
    {
        newB.append(Y(2,i));
    }

    // 将三个波段组合起来
    int bytePerLine = (high_w * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * high_h];
    for (int h = 0; h < high_h; h++)
    {
        for (int w = 0; w < high_w; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = newR[h * high_w + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = newG[h * high_w + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = newB[h * high_w + w];
        }
    }

    image=new QImage(allBandUC, high_w, high_h, bytePerLine, QImage::Format_RGB888);
    image->save("PCA.jpg");

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
//    // 标准化处理 计算均值向量M和协方差矩阵C
//    for(int i = 0;i < X.length();i++)
//    {
//        int xj = 0;
//        int varxj = 0;
//        for(int j =0;j < X[i].length();j++)
//        {
//            xj += X[i][j];
//        }
//        xj = xj / X[0].length();
//        for(int j = 0;j < X[i].length();i++)
//        {
//            varxj += qPow(X[i][j] - xj ,2);
//        }
//        varxj = varxj / (X[0].length() - 1);
//        for(int j = 0;j < X[i].length();j++)
//        {
//            X[i][j] = (X[i][j] - xj) / qSqrt(varxj);
//        }
//    }
//    // 相关系数矩阵
//    QVector<QVector<float>> R;
//    R.resize(3);
//    for(int i = 0;i < R.length();i++)
//    {
//       R[i].resize(high_h * high_w);
//    }
//    for(int i = 0;i < R.length();i++)
//    {
//       int tempr = 1;
//       for(int j = 0;j < R[i].length();j++)
//       {
//            tempr = tempr * X[i][i] * X[i][j];
//       }
//    }
    // 雅可比行列式求得特征值和对应的特征向量
    // 将特征值排序
    // 根据变换方程计算各主分量
    // 全色影像和第一主分量进行直方图匹配
    // 逆变换得到全新影像

}
#pragma endregion}

#pragma endregion}

#pragma region"非监督分类"{

#pragma region"Kmeans聚类"{
void operation::KMEANS()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299);
    }

    // 手动设置迭代次数以及截止精度
    int itercnt = 5;
    float eps = 0.50;
    int maxdiedai = 20;

    // 初始化质心位置
    QVector<float> label;
    label.resize(itercnt);
    int m = 0;
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        bool k =false;
        float temp = buffer[i];
        for(int j = 0;j < itercnt;j++)
        {
            if(temp == label[j])
            {
                k=true;
                break;
            }
        }
        if(!k)
            label[m++] = temp;
        if(m == itercnt)
            break;
    }

    QVector<QVector<float>> L;
    L.resize(itercnt);
    QVector<QVector<float>> fenlei;
    fenlei.resize(2);
    for(int i = 0;i < fenlei.length();i++)
    {
        fenlei[i].resize(iScaleHeight * iScaleWidth);
    }
    float var0 = 0;

    for(int m = 0;m < maxdiedai;m++)
    {
        // 开始聚类A
        float minDis = 9999;
        int mark = -1;
        for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
        {
            for(int j = 0;j < itercnt;j++)
            {
                float temp = qSqrt(qPow(buffer[i] - label[j],2));
                if(temp < minDis)
                {
                    minDis = temp;
                    mark = j;
                }
            }
//            fenlei[0][i] = (buffer[i]);
//            fenlei[1][i] = mark;
            if (mark == 0)
            {
                L[0].append(buffer[i]);
                fenlei[0][i] = (buffer[i]);
                fenlei[1][i] = 0;
            }
            if (mark == 1)
            {
                L[1].append(buffer[i]);
                fenlei[0][i] = (buffer[i]);
                fenlei[1][i] = 1;
            }
            if (mark == 2)
            {
                L[2].append(buffer[i]);
                fenlei[0][i] = (buffer[i]);
                fenlei[1][i] = 2;
            }
            if (mark == 3)
            {
                L[3].append(buffer[i]);
                fenlei[0][i] = (buffer[i]);
                fenlei[1][i] = 3;
            }
            if (mark == 4)
            {
                L[4].append(buffer[i]);
                fenlei[0][i] = (buffer[i]);
                fenlei[1][i] = 4;
            }
            minDis = 9999;
        }

        // 计算新的质心
        for(int i = 0;i < itercnt;i++)
        {
            float sum = 0;
            for(int j = 0;j < L[i].length();j++)
            {
                sum += L[i][j];
            }
            label[i] = sum / L[i].length();
        }

        // 计算每个类别中数值的方差值，然后求出所有类别方差值得均值var，将var作为一个判别准则，
        // 当本次var与上次var之间的变化小于eps时，或者迭代次数大于iterCnt时，停止迭代
        QVector<float> fangcha;
        for(int i = 0;i < itercnt;i++)
        {
            float sum = 0;
            for(int j = 0;j < L[i].length();j++)
            {
                sum += qPow(L[i][j] - label[i],2);
            }
            fangcha.append(qSqrt(sum / (L[i].length() - 1)));
        }
        float var = 0;
        for(int i = 0;i < fangcha.length();i++)
        {
            var += fangcha[i];
        }
        var = var / fangcha.length();

        if(m == maxdiedai || abs((var - var0) / var) < eps)
        {
            break;
        }
        var0 = var;
    }

    // 对每个类别里的灰度值在原来的rgb数组中重新赋值
    for(int i = 0;i < iScaleHeight * iScaleWidth;i++)
    {
        int ma = fenlei[1][i];
        fenlei[0][i] = (int)((ma + 1) * 255 / itercnt);
    }

    QVector<float> newR;
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        newR.append((int)fenlei[0][i]);
    }
    QVector<float> newG = newR;
    QVector<float> newB = newR;

    // 显示图像
    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = newR[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = newG[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = newB[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma region"ISODATA聚类"{
void operation::ISODATA()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299);
    }
    delete[] rBand;
    delete[] gBand;
    delete[] bBand;

    // 初始化变量
    int K = 7;                  // 期望得到的聚类数
    int k = 5;                  // 初始的设定的聚类数
    int N0 = 10000;             // 每一个类别中最少的样本数，若少于此数则去掉该类别
    float s = 0.0001f;              // 一个类别中，样本特征中最大标准差。若大于这个值，则可能分裂
    float c = 0.0005f;              // 两个类别中心间的最小距离，若小于此数，把两个类别需进行合并
    // int L = 4;                  // 在一次合并操作中，可以合并的类别的最多对数
    int I = 5;                 // 迭代运算的次数

    QVector<QVector<float>> result; // [类别，灰度值]
    QVector<QVector<float>> result0;
    // 从这里开始迭代
    int i = 0;
    do
    {
        // 初始化k个质心
        QVector<float> label;
        label.resize(k);
        int m = 0;
        for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
        {
            bool kk =false;
            float temp = buffer[i];
            for(int j = 0;j < k;j++)
            {
                if(temp == label[j])
                {
                    kk=true;
                    break;
                }
            }
            if(!kk)
                label[m++] = temp;
            if(m == k)
                break;
        }
        result0 = julei0(buffer,k,label);
        result = julei(buffer,k,label);
        // 如果每类的样本数目小于N0，则取消该样本子集，此时k减去1
        // 保证第一次聚类的结果每一类的数量都在设定值之上
        for(int j = 0;j < k;j++)
        {
            if(result[j].length() < N0)
            {
                k = k - 1;
                result = julei(buffer,k,label);
                result0 = julei0(buffer,k,label);
            }
        }

        // 更新聚类中心
        for(int i = 0;i < k;i++)
        {
            float sum = 0;
            for(int j = 0;j < result[i].length();j++)
            {
                sum += result[i][j];
            }
            label[i] = sum / result[i].length();
        }

        // 1、计算每个类的类内平均距离
        // 2、计算所有类的总体平均距离
        QVector<float> distance; // 样本与各聚类中心间的平均距离
        float D = 0; // 全部样本和其对应聚类中心的总平均距离
        for(int i = 0;i < k;i++)
        {
            float d = 0;
            for(int j = 0;j < result[i].length();j++)
            {
                d += qSqrt(qPow(result[i][j] - label[i],2));
            }
            D += d;
            distance.append(d / result[i].length());
        }
        D = D / (iScaleHeight * iScaleWidth);

        if(k < (int)K/2) // 分裂处理
        {
            k = fenlie(result,label,s,k);
        }
        else  // 合并处理
        {
            if(k > 2*K || i%2 == 0)
                k = hebing(result,label,c,k);
            else
                k = fenlie(result,label,s,k);
        }
        i++;
    }while(i < I);

    QVector<float> bandvalue;
    for(int i = 0;i < iScaleHeight * iScaleWidth;i++)
    {
        int ma = result0[1][i];
        bandvalue.append((int)((ma + 1) * 255 / k));
    }

    delete[] buffer;

    // 显示图像
    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = bandvalue[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = bandvalue[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bandvalue[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}

QVector<QVector<float>> operation::julei(float *buffer,int k,QVector<float> label)
{
    QVector<QVector<float>> result;
    result.resize(k);
    QVector<float> fenlei;
    fenlei.resize(k);
    // 聚类
    for(int m = 0;m < iScaleHeight * iScaleWidth;m++)
    {
        float tempmin = 9999;
        int mark = -1;
        for(int n = 0;n < k;n++)
        {
            float distance = qSqrt(qPow(buffer[m] - label[n] ,2));
            if(tempmin > distance)
            {
                tempmin = distance;
                mark = n;
            }
        }
        result[mark].append(tempmin);
    }
    return result;
}

QVector<QVector<float>> operation::julei0(float *buffer, int k, QVector<float> label)
{
    QVector<QVector<float>> result0;
    result0.resize(2);
    for(int i = 0;i < result0.length();i++)
    {
        result0[i].resize(iScaleHeight * iScaleWidth);
    }
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        float minDis = 9999;
        int mark0 = -1;
        for(int j = 0;j < k;j++)
        {
            float temp = qSqrt(qPow(buffer[i] - label[j],2));
            if(temp < minDis)
            {
                minDis = temp;
                mark0 = j;
            }
        }
        result0[0][i] = (buffer[i]);
        result0[1][i] = mark0;
    }
    return result0;
}

int operation::fenlie(QVector<QVector<float>> result,QVector<float> label,float s,int k)
{
    /*计算需要分裂的这个簇在各个维度上的方差，
    * 如果最大的方差超过了特定的阈值，就在这个最大方差的维度上分裂成两个，
    * 其他维度的值保持不变*/
    /*1、 result为目前聚类结果，
     *2、label为每个聚类的中心值，
     *3、s为方差的阈值
     *4、k为目前聚类的总数*/
    QVector<float> fangcha;
    for(int i = 0;i < result.length();i++)
    {
        float temp_fangcha = 0;
        for(int j = 0;j < result[i].length();j++)
        {
            temp_fangcha += qPow(result[i][j] - label[i],2);
        }
        fangcha.append(qSqrt(temp_fangcha) / result[i].length());
    }

    // 判断方差是否超过了阈值
    float max = -1;
    for(int i = 0;i < fangcha.length();i++)
    {
        if (fangcha[i] > max)
            max = fangcha[i];
    }
    if(max > s)
        k = k + 1;

    return k;
}

int operation::hebing(QVector<QVector<float>> result,QVector<float> label,float c,int k)
{
    float min = 9999;
    float dis = 0;
    for(int i = 0;i < k;i++)
    {
        for(int j = i+1;j < k;j++)
        {
            dis = qSqrt(qPow(label[i] - label[j],2));
            if (min > dis)
                min = dis;
        }
    }
    if (min > c)
        k = k - 1;

    return k;
}
#pragma endregion}

#pragma region"高斯混合聚类"{
double operation::gauss(const double x, const double m, const double sigma)
{
    return 1.0 / (qSqrt(2 * 3.1415926)*sigma)*qExp(-0.5*(x - m)*(x - m) / (sigma*sigma));
}

void operation::GaussianMixtureModel()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);

    // 加权对三个通道的灰度值进行合成一个通道
    QVector<float> buffer;
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer.append((int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299));
    }

    delete[] rBand;
    delete[] gBand;
    delete[] bBand;

    // 获取输入数据
    QVector<float> data = buffer;
    int dataNum = data.size();
    // 存储最终需要的结果
    int clusterNum = 7;        // 聚类数量
    double epslon = 0.5;                 // 阈值
    double max_steps = 20;        // 最大的迭代次数
    // 保留每一个类别的均值，方差参数，保留上一个的参数
    QVector<float> means;
    means.resize(clusterNum);
    QVector<float> means_bkp; // 上一次的迭代数据
    means_bkp.resize(clusterNum);
    QVector<float> sigmas;
    sigmas.resize(clusterNum);
    QVector<float> sigmas_bkp; // 上一次的迭代数据
    sigmas_bkp.resize(clusterNum);
    // 保留每一个数据对于每一个类别下的概率，由在这个类别下的概率除以到各个类别的总概率得到
    QVector<QVector<float>> memberships; // 存储属于哪一个类别
    memberships.resize(clusterNum);
    QVector<QVector<float>> memberships_bkp;
    memberships_bkp.resize(clusterNum);
    for (int i = 0; i < clusterNum; i++)
    {
        memberships[i].resize(data.size());
        memberships_bkp[i].resize(data.size());
    }
    // 每一个类别的可能性
    QVector<float> probilities;
    probilities.resize(clusterNum);
    QVector<float> probilities_bkp;
    probilities_bkp.resize(clusterNum);
    //initialize mixture probabilities 初始化每个类别的参数
    for (int i = 0; i < clusterNum; i++)
    {
        probilities[i] = probilities_bkp[i] = 1.0 / (float)clusterNum;
        //init means
        means[i] = means_bkp[i] = 255.0*i / (clusterNum);
        //init sigma
        sigmas[i] = sigmas_bkp[i] = 50;
    }

    //compute membership probabilities
    int i, j, k, m;
    double sum = 0, sum2;
    int steps = 0;
    bool go_on;
    do          // 迭代
    {
        for (k = 0; k < clusterNum; k++)
        {
            //compute membership probabilities
            for (j = 0; j < data.size(); j++)
            {
                //计算p(k|n)，计算每一个数据在每一个类别的值的加权和
                sum = 0;
                for (m = 0; m < clusterNum; m++)
                {
                    sum += probilities[m] * gauss(data[j], means[m], sigmas[m]);
                }
                //求分子，第j个数据在第k类中的所占的比例
                memberships[k][j] = probilities[k] * gauss(data[j], means[k], sigmas[k]) / sum;
            }
            //求均值
            //求条件概率的和，将每个数据在第k类中的概率进行累加
            sum = 0;
            for (i = 0; i < dataNum; i++)
            {
                sum += memberships[k][i];
            }
            //得到每个数据在属于第k类的加权值之和
            sum2 = 0;
            for (j = 0; j < dataNum; j++)
            {
                sum2 += memberships[k][j] * data[j];
            }
            //得到新的均值 由概率加权和除以总概率作为均值
            means[k] = sum2 / sum;
            //求方差  由到均值的平方的加权和作为新的方差
            sum2 = 0;
            for (j = 0; j < dataNum; j++)
            {
                sum2 += memberships[k][j] * (data[j] - means[k])*(data[j] - means[k]);
            }
            sigmas[k] = sqrt(sum2 / sum);
            //求概率
            probilities[k] = sum / dataNum;
        }//end for k
        //check improvement
        go_on = false;
        for (k = 0; k<clusterNum; k++)
        {
            if (means[k] - means_bkp[k]>epslon)
            {
                go_on = true;
                break;
            }
        }
        //back up
        means_bkp = means;
        sigmas_bkp = sigmas;
        probilities_bkp = probilities;
    } while (go_on&&steps++ < max_steps); //end do while

    QVector<float> bandvalue;
    for(int i = 0;i < memberships[0].length();i++)
    {
        float tempmax = -1;
        int mark = -1;
        for(int j = 0;j < memberships.length();j++)
        {
            if(tempmax < memberships[j][i])
            {
                mark = j;
                tempmax = memberships[j][i];
            }
        }
        bandvalue.append((int)(255 * (mark + 1) / clusterNum));
    }

    // 显示图像
    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = bandvalue[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = bandvalue[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = bandvalue[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma endregion}

#pragma region"特征提取"{

#pragma region"Harris算子"{
void operation::HARRIS()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299);
    }

    delete[] rBand;
    delete[] gBand;
    delete[] bBand;
    // float*转成mat类型
    Mat gray(iScaleHeight, iScaleWidth, CV_8U);
    for(int i = 0;i < iScaleHeight;i++)
    {
        for(int j = 0;j < iScaleWidth;j++)
        {
            gray.at<uchar>(i, j) = static_cast<uchar>(buffer[i * iScaleWidth + j]);
        }
    }
    delete[] buffer;
    //计算Harris系数
    Mat harris;
    int blockSize = 2;     // 邻域半径
    int apertureSize = 3;  // 邻域大小
    cv::cornerHarris(gray, harris, blockSize, apertureSize, 0.04);
    //归一化便于进行数值比较和结果显示
    Mat harrisn;
    normalize(harris, harrisn, 0, 255, NORM_MINMAX);
    //将图像的数据类型变成CV_8U
    convertScaleAbs(harrisn, harrisn);
    //寻找Harris角点
    std::vector<cv::KeyPoint> keyPoints;
    for (int row = 0; row < harrisn.rows; row++)
    {
        for (int col = 0; col < harrisn.cols; col++)
        {
            int R = harrisn.at<uchar>(row, col);
            if (R > 90)
            {
                //向角点存入KeyPoint中
                KeyPoint keyPoint;
                keyPoint.pt.y = row;
                keyPoint.pt.x = col;
                keyPoint.size = 5.0;
                keyPoints.push_back(keyPoint);
            }
        }
    }
    // 复制输入图像
    Mat outputImage;
    // 绘制角点与显示结果
    if (!keyPoints.empty() && !gray.empty())
    {
        try
        {
            drawKeypoints(gray, keyPoints, outputImage, Scalar(255, 0, 0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        }
        catch (const std::exception& e)
        {
            qDebug()<< "Exception: " << e.what();
        }
    }
    else
    {
        // 如果没有检测到关键点或图像为空，使用红色或其他颜色来绘制
        drawKeypoints(gray, keyPoints, outputImage, Scalar(0, 0, 255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    }
    // 显示在qimage控件中 Mat转Qimage
    QImage *img2 =new QImage((const unsigned char*)(outputImage.data),outputImage.cols,outputImage.rows,outputImage.step,QImage::Format_RGB888);
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*img2));

    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma region"MORAVEC算子"{
void operation::MORAVEC()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299);
    }

    delete[] rBand;
    delete[] gBand;
    delete[] bBand;

    // float*转成mat类型 三通道转成灰度单通道进行处理
    Mat gray(iScaleHeight, iScaleWidth, CV_8U); // CV_8U的数据类型有利于匹配后续的函数传参

    for (int i = 0; i < iScaleHeight; i++)
    {
        for (int j = 0; j < iScaleWidth; j++)
        {
            // 将浮点数转换为uchar类型，适应CV_8U数据类型
            gray.at<uchar>(i, j) = static_cast<uchar>(buffer[i * iScaleWidth + j]);
        }
    }

    delete[] buffer;
    //窗口设置为5*5，阈值设置为5000
    Mat mat_gray = MoravecCorners(gray, 5, 5000);
    // 显示在qimage控件中 Mat转Qimage
    QImage *img2 =new QImage((const unsigned char*)(mat_gray.data),mat_gray.cols,mat_gray.rows,mat_gray.step,QImage::Format_RGB888);
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*img2));
    myscene->addItem(imgItem);
}

Mat operation::MoravecCorners(Mat SrcImage, int kSize, int threshold)
{
    Mat mat_gray = SrcImage.clone();
    // 获取初始化参数信息
    int r = kSize / 2;
    // 存储特征点
    std::vector<cv::KeyPoint> keyPoints;

    // 图像遍历
    for (int y = r; y < SrcImage.rows - r; y++)//行（y轴--v轴）
    {
        for (int x = r; x < SrcImage.cols - r; x++)//列（x轴--u轴）
        {
            int wV1, wV2, wV3, wV4;
            wV1 = wV2 = wV3 = wV4 = 0;
            // 计算水平(0度）方向窗内兴趣值
            for (int k = -r; k < r; k++)
                wV1 += (SrcImage.at<uchar>(y, x + k) - SrcImage.at<uchar>(y, x + k + 1))*
                       (SrcImage.at<uchar>(y, x + k) - SrcImage.at<uchar>(y, x + k + 1));
            // 计算垂直(90度）方向窗内兴趣值
            for (int k = -r; k < r; k++)
                wV2 += (SrcImage.at<uchar>(y + k, x) - SrcImage.at<uchar>(y + k + 1, x))*
                       (SrcImage.at<uchar>(y + k, x) - SrcImage.at<uchar>(y + k + 1, x));
            // 计算45度方向窗内兴趣值
            for (int k = -r; k < r; k++)
                wV3 += (SrcImage.at<uchar>(y + k, x + k) - SrcImage.at<uchar>(y + k + 1, x + k + 1))*
                       (SrcImage.at<uchar>(y + k, x + k) - SrcImage.at<uchar>(y + k + 1, x + k + 1));
            // 计算135度方向窗内兴趣值
            for (int k = -r; k < r; k++)
                wV4 += (SrcImage.at<uchar>(y + k, x - k) - SrcImage.at<uchar>(y + k + 1, x - k - 1))*
                       (SrcImage.at<uchar>(y + k, x - k) - SrcImage.at<uchar>(y + k + 1, x - k - 1));
            // 取其中的最小值作为该像素点的最终兴趣值
            int value = min(min(wV1, wV2), min(wV3, wV4));

            // 若兴趣值大于阈值，则将关键点存入向量中
            if (value > threshold)
            {
                keyPoints.push_back(KeyPoint(x, y, 5));
            }
        }
    }
    drawKeypoints(SrcImage, keyPoints, mat_gray, Scalar(255, 0, 0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    return mat_gray;
}
#pragma endregion}

#pragma region"直线检测Hough变换"{
void operation::LineDetection()
{
    QString fileName = QFileDialog::getOpenFileName(0,QFileDialog::tr("Open Image"),".",QFileDialog::tr("Image File(*.png *.jpg *.jpeg *.tif *.img)"));
    QFile file(fileName);

    if(!file.open(QFile::ReadOnly))
    {
        qDebug("读取失败");
    }
    else{
        QByteArray ba = file.readAll();
        image1 = imdecode(std::vector<char>(ba.begin(), ba.end()), 1);
    }
    if(image1.empty ()){
        qDebug("图像为空");
    }
    else
    {
        cvtColor(image1,image1,COLOR_BGR2RGB);
        Mat mat_gray;

        cvtColor(image1, mat_gray, cv::COLOR_BGR2GRAY);

        Mat mat_draw = image1.clone();  // 创建原始图像的副本
        Mat mat_binary;
        Mat mat_canny;

        //形态学闭运算
        Mat elementRect;
        elementRect = getStructuringElement(MORPH_RECT,Size(3,3),Point(-1,-1));
        morphologyEx(mat_gray,mat_gray,MORPH_CLOSE,elementRect);

        // binary
        threshold(mat_gray, mat_binary, 125, 255.0, THRESH_BINARY);

        // detect edge
        Canny(mat_binary, mat_canny, 50, 150, 3);

        // detect line
//        std::vector<cv::Vec2f> lines;
//        HoughLines(mat_canny, lines, 1, CV_PI / 180, 150, 0, 0);

//        // draw line
//        for (size_t i = 0; i < lines.size(); i++)
//        {
//            float rho = lines[i][0], theta = lines[i][1];
//            Point pt1, pt2;
//            double a = qCos(theta), b = qSin(theta);
//            double x0 = a * rho, y0 = b * rho;
//            pt1.x = cvRound(x0 + 1000 * (-b));
//            pt1.y = cvRound(y0 + 1000 * (a));
//            pt2.x = cvRound(x0 - 1000 * (-b));
//            pt2.y = cvRound(y0 - 1000 * (a));
//            line(mat_draw, pt1, pt2, Scalar(0, 255, 0), 10);
//        }
        std::vector<Vec4f> lines;
        HoughLinesP(mat_canny,lines, 1, CV_PI / 180, 50, 0, 10);
        // 在原始图像上绘制直线
        for (size_t i = 0; i < lines.size(); ++i)
        {
            Point pt1(cvRound(lines[i][0]), cvRound(lines[i][1]));
            Point pt2(cvRound(lines[i][2]), cvRound(lines[i][3]));
            line(mat_draw, pt1, pt2, Scalar(0, 0, 255), 10);
        }

        // 显示在qimage控件中 Mat转Qimage
        QImage *img2 =new QImage((const unsigned char*)(mat_draw.data),mat_draw.cols,mat_draw.rows,mat_draw.step,QImage::Format_RGB888);
        QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*img2));
        myscene->addItem(imgItem);
    }
}
#pragma endregion}

#pragma endregion}

#pragma region"影像分割"{

#pragma region"迭代阈值分割"{
void operation::IterativeThresholdSegmentation()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299);
    }
    delete[] rBand;
    delete[] gBand;
    delete[] bBand;

    // 选取图像灰度范围的中值作为初始估计值T
    QVector<QVector<float>> band; // 每个灰度值及其对应的频数，升序排序
    band=PaiXu(SataticsBandsValue(buffer,iScaleWidth * iScaleHeight)); // [灰度值，频数]
    QVector<float> maxmin;
    maxmin = GetHistormMAXMin(buffer,iScaleHeight * iScaleWidth);
    float zhongwei = iScaleHeight * iScaleHeight / 2;
    float sum = 0;
    float T = 0;
    for(int i = 0;i < band[0].length();i++)
    {
        sum += band[1][i];
        if(sum > zhongwei)
        {
            T = band[0][i];
            break;
        }
    }
    float T0 = (maxmin[0] + maxmin[1]) / 2;
    // 用T分割图像。这样便会生成两组像素集合：G1由所有灰度值大于T的像素组成，而G2由所有灰度值小于或等于T的像素组成。
    // 对G1和G2中所有像素计算平均灰度值1和2
    while(qAbs(T - T0) > 0.5)
    {
        T0 =T;
        QVector<float> G1;
        QVector<float> G2;
        float sumG1 = 0;
        float sumG2 = 0;
        float averageG1 = 0;
        float averageG2 = 0;
        for(int i = 0;i < iScaleHeight * iScaleWidth;i++)
        {
            if(buffer[i] > T)
            {
                sumG1 += buffer[i];
                G1.append(buffer[i]);
            }
            else
            {
                sumG2 += buffer[i];
                G2.append(buffer[i]);
            }
        }
        averageG1 = sumG1 / G1.length();
        averageG2 = sumG2 / G2.length();
        // 计算新的阈值：T= (1+2) /2。重复步骤 2）到 4），直到得到的T值之差小于一个事先定义的参数T
        T = (averageG1 + averageG2) / 2;
    }

    QVector<float> newBand;
    for(int i =0;i < iScaleHeight * iScaleWidth;i++)
    {
        if(buffer[i] > (int)T)
            newBand.append(255);
        else
            newBand.append(0);
    }
    delete[] buffer;

    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = newBand[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = newBand[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = newBand[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma region"OSTU"{
void operation::OSTU()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);

    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299);
    }
    delete[] rBand;
    delete[] gBand;
    delete[] bBand;

    QVector<QVector<float>> sigma;    // 类间方差
    sigma.resize(2);
    // 阈值最终为0-255的其中一个数值，根据此结果直接对阈值作256次迭代，取最高的一次作为最终结果
    for(int k = 0;k < 255;k++)
    {
        // 前景和背景
        QVector<float> front;
        QVector<float> back;
        float sumfront = 0; // 前景的像素总数
        float sumback = 0;  // 背景的像素总数
        float meanfront = 0; // 前景的像素平均值
        float meanback = 0;  // 背景的像素平均值
        float pfront = 0;   // 前景的像素概率
        float pback = 0;    // 背景的像素概率
        for(int i = 0;i < iScaleHeight * iScaleWidth;i++)
        {
            if(buffer[i] > k)
            {
                sumfront += buffer[i];
                front.append(buffer[i]);
            }
            else
            {
                sumback += buffer[i];
                back.append(buffer[i]);
            }
        }
        meanfront = sumfront / front.length();
        meanback = sumback / back.length();
        pfront = sumfront / (iScaleHeight * iScaleWidth);
        pback = sumback / (iScaleHeight * iScaleWidth);

        sigma[0].append(1.0 * k);
        sigma[1].append(pfront * pback * qPow(meanfront - meanback,2));
    }

    float value = 0;
    float max = sigma[1][0];
    for(int i = 1;i < sigma[0].length();i++)
    {
        if(max < sigma[1][i])
        {
            max = sigma[1][i];
            value = sigma[0][i];
        }
    }

    QVector<float> newBand;
    for(int i =0;i < iScaleHeight * iScaleWidth;i++)
    {
        if(buffer[i] > (int)value)
            newBand.append(255);
        else
            newBand.append(0);
    }
    delete[] buffer;

    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = newBand[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = newBand[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = newBand[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);
    //SaveRaster(*image);
    SaveImageWithGDAL(allBandUC,iScaleWidth,iScaleHeight);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma endregion}

#pragma region"监督分类"{

#pragma region"最大似然法"{
void operation::MLC()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);
    int Bandnum = 3; // 影像波段数
    int centernum = 5; // 最大似然分类数为5类
    QVector<QVector<float>> traindata;
    traindata.resize(Bandnum);
    for (int i = 0; i < Bandnum; i++) // 存入train3个波段数据
    {
        for(int j = 0;j < iScaleHeight * iScaleWidth;j++)
        {
            switch (i) {
            case 0:
                traindata[i].append(rBand[j]);
                break;
            case 1:
                traindata[i].append(gBand[j]);
                break;
            case 2:
                traindata[i].append(bBand[j]);
                break;
            default:
                break;
            }
        }
    }

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299);
    }

    QVector<float> trainlabeldata; // train标签图数据
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        trainlabeldata.append(buffer[i]);
    }

    QVector<QVector<float>> train; // 用于存放五类数据
    train.resize(centernum);
    for (int i = 0; i < centernum; i++)
    {
        train[i].resize(Bandnum);
    }

    for (int i = 0; i < centernum; i++)
    {
        for (int j = 0; j < Bandnum; j++)
        {
            train[i][j] = 0;
        }
    }

    QVector<int> count;
    count.resize(centernum);//计数
    for (int i = 0; i < centernum; i++)
    {
        count[i] = 0;
    }

    // 分波段分别存放五类数据
    for (int i = 0; i < iScaleWidth * iScaleHeight; i++)
    {
        int n = 0;
        if (trainlabeldata[i] == 50)
        {
            n = 1;
        }
        if (trainlabeldata[i] == 100)
        {
            n = 2;
        }
        if (trainlabeldata[i] == 150)
        {
            n = 3;
        }
        if (trainlabeldata[i] == 200)
        {
            n = 4;
        }

        for (int j = 0; j < Bandnum; j++)
        {
            train[n][j] += int(traindata[j][i]);
            count[n]++;
        }
    }

    // 计算各类样本均值向量
    MatrixXd ECombined(Bandnum, centernum);
    VectorXd E1(Bandnum);
    VectorXd E2(Bandnum);
    VectorXd E3(Bandnum);
    VectorXd E4(Bandnum);
    VectorXd E5(Bandnum);
    for (int i = 0; i < Bandnum; i++)
    {
        E1(i) = train[0][i] / (count[0] / 3);
        E2(i) = train[1][i] / (count[1] / 3);
        E3(i) = train[2][i] / (count[2] / 3);
        E4(i) = train[3][i] / (count[3] / 3);
        E5(i) = train[4][i] / (count[4] / 3);
    }
    for (int j = 0; j < centernum; j++)
    {
        for (int i = 0; i < Bandnum; i++)
        {
            ECombined(i, j) = train[j][i] / (count[j] / 3);
        }
    }

    // 计算各类协方差矩阵
    MatrixXd sigma1 = MatrixXd::Zero(Bandnum, Bandnum);
    MatrixXd sigma2 = MatrixXd::Zero(Bandnum, Bandnum);
    MatrixXd sigma3 = MatrixXd::Zero(Bandnum, Bandnum);
    MatrixXd sigma4 = MatrixXd::Zero(Bandnum, Bandnum);
    MatrixXd sigma5 = MatrixXd::Zero(Bandnum, Bandnum);

    for (int i = 0; i < Bandnum; i++)
    {
        for (int j = 0; j < Bandnum; j++)
        {
            for (int n = 0; n < iScaleWidth*iScaleHeight; n++)
            {
                if (trainlabeldata[n] == 0)
                {

                    sigma1(i, j) += (traindata[i][n] - E1(i))*(traindata[j][n] - E1(j));
                }
                if (trainlabeldata[n] == 50)
                {
                    sigma2(i, j) += (traindata[i][n] - E2(i))*(traindata[j][n] - E2(j));
                }
                if (trainlabeldata[n] == 100)
                {
                    sigma3(i, j) += (traindata[i][n] - E3(i))*(traindata[j][n] - E3(j));
                }
                if (trainlabeldata[n] == 150)
                {
                    sigma4(i, j) += (traindata[i][n] - E4(i))*(traindata[j][n] - E4(j));
                }
                if (trainlabeldata[n] == 200)
                {
                    sigma5(i, j) += (traindata[i][n] - E5(i))*(traindata[j][n] - E5(j));
                }

            }
            sigma1(i, j) = sigma1(i, j) / (count[0] / 3);
            sigma2(i, j) = sigma2(i, j) / (count[1] / 3);
            sigma3(i, j) = sigma3(i, j) / (count[2] / 3);
            sigma4(i, j) = sigma4(i, j) / (count[3] / 3);
            sigma5(i, j) = sigma5(i, j) / (count[4] / 3);
        }
    }

    MatrixXd sigmaCombined(Bandnum * centernum, Bandnum); // 矩阵形式存放协方差便于后续使用
    sigmaCombined.block(0, 0, Bandnum, Bandnum) = sigma1;
    sigmaCombined.block(Bandnum, 0, Bandnum, Bandnum) = sigma2;
    sigmaCombined.block(2 * Bandnum, 0, Bandnum, Bandnum) = sigma3;
    sigmaCombined.block(3 * Bandnum, 0, Bandnum, Bandnum) = sigma4;
    sigmaCombined.block(4 * Bandnum, 0, Bandnum, Bandnum) = sigma5;

    unsigned char** testdata = new unsigned char*[Bandnum];
    for (int i = 0; i < Bandnum; i++) // 存入test3个波段数据
    {
        testdata[i] = new unsigned char[iScaleWidth*iScaleHeight];
        for(int j = 0;j < iScaleHeight * iScaleWidth;j++)
        {
            switch (i) {
            case 0:
                testdata[i][j] = rBand[j];
                break;
            case 1:
                testdata[i][j] = gBand[j];
                break;
            case 2:
                testdata[i][j] = bBand[j];
                break;
            default:
                break;
            }
        }
    }
    int* label = new int[iScaleWidth*iScaleHeight];//标签

    // 计算各类概率密度函数对待分类样本进行分类
    for (int n = 0; n < iScaleWidth*iScaleHeight; n++)
    {
        VectorXd X(Bandnum);
        VectorXd E(Bandnum);
        MatrixXd sigma(Bandnum, Bandnum);
        double P = 0.0;
        int temp = 0;
        for (int i = 0; i < Bandnum; i++)
        {
            X(i) = testdata[i][n];
        }
        for (int i = 0; i < centernum; i++)
        {
            E = ECombined.col(i);
            sigma = sigmaCombined.block(i*Bandnum, 0, Bandnum, Bandnum);
            double PXY = (1 / pow(2 * 3.14, Bandnum / 2))*(1 / pow(sigma.determinant(), 0.5))*exp((-0.5)*((X - E).transpose())*sigma.inverse()*(X - E));
            if (PXY > P)
            {
                P = PXY;
                temp = i;
            }
        }
        label[n] = temp;
    }

    // 输出分类结果
    unsigned char* output = new unsigned char[iScaleWidth*iScaleHeight];

    for (int i = 0; i < iScaleWidth*iScaleHeight; i++)
    {
        if (label[i] == 0)
        {
            output[i] = 0;
        }
        if (label[i] == 1)
        {
            output[i] = 50;
        }
        if (label[i] == 2)
        {
            output[i] = 100;
        }
        if (label[i] == 3)
        {
            output[i] = 150;
        }
        if (label[i] == 4)
        {
            output[i] = 200;
        }
    }

    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = output[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = output[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = output[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma region"最小距离法"{
void operation::MinDistance()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * gBand = new float[iScaleWidth * iScaleHeight];
    float * bBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, w, h, gBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, bBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    // 线性拉伸0-255
    rBand = Stretch255(rBand,rasterDataset->GetRasterBand(1),iScaleWidth * iScaleHeight);
    gBand = Stretch255(gBand,rasterDataset->GetRasterBand(2),iScaleWidth * iScaleHeight);
    bBand = Stretch255(bBand,rasterDataset->GetRasterBand(3),iScaleWidth * iScaleHeight);
    int Bandnum = 3;
    int centernum = 5;
    QVector<QVector<float>> traindata;
    traindata.resize(Bandnum);
    for (int i = 0; i < Bandnum; i++) // 存入train3个波段数据
    {
        for(int j = 0;j < iScaleHeight * iScaleWidth;j++)
        {
            switch (i) {
            case 0:
                traindata[i].append(rBand[j]);
                break;
            case 1:
                traindata[i].append(gBand[j]);
                break;
            case 2:
                traindata[i].append(bBand[j]);
                break;
            default:
                break;
            }
        }
    }

    // 加权对三个通道的灰度值进行合成一个通道
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)(rBand[i] * 0.114 + gBand[i] * 0.587 + bBand[i] * 0.299);
    }

    QVector<float> trainlabeldata; // train标签图数据
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        trainlabeldata.append(buffer[i]);
    }

    QVector<int> count;
    count.resize(centernum);//计数
    for (int i = 0; i < centernum; i++)
    {
        count[i] = 0;
    }

    QVector<QVector<float>> train; // 用于存放五类数据
    train.resize(centernum);
    for (int i = 0; i < centernum; i++)
    {
        train[i].resize(Bandnum);
    }

    for (int i = 0; i < centernum; i++)
    {
        for (int j = 0; j < Bandnum; j++)
        {
            train[i][j] = 0;
        }
    }

    // 分波段分别存放五类数据
    for (int i = 0; i < iScaleWidth * iScaleHeight; i++)
    {
        int n = 0;
        if (trainlabeldata[i] == 50)
        {
            n = 1;
        }
        if (trainlabeldata[i] == 100)
        {
            n = 2;
        }
        if (trainlabeldata[i] == 150)
        {
            n = 3;
        }
        if (trainlabeldata[i] == 200)
        {
            n = 4;
        }

        for (int j = 0; j < Bandnum; j++)
        {
            train[n][j] += int(traindata[j][i]);
            count[n]++;
        }
    }

    QVector<QVector<float>> center;
    center.resize(centernum); // 用于存放中心向量
    for (int i = 0; i < centernum; i++)
    {
        center[i].resize(Bandnum);
    }

    for (int i = 0; i < centernum; i++)
    {
        for (int j = 0; j < Bandnum; j++)
        {
            center[i][j] = 0.0;
        }
    }

    // 计算中心向量
    for (int i = 0; i < centernum; i++)
    {
        for (int j = 0; j < Bandnum; j++)
        {
            center[i][j] = double(train[i][j] / (count[i]/3));
        }
    }

    unsigned char** testdata = new unsigned char*[Bandnum];
    for (int i = 0; i < Bandnum; i++) // 存入test3个波段数据
    {
        testdata[i] = new unsigned char[iScaleWidth*iScaleHeight];
        for(int j = 0;j < iScaleHeight * iScaleWidth;j++)
        {
            switch (i) {
            case 0:
                testdata[i][j] = rBand[j];
                break;
            case 1:
                testdata[i][j] = gBand[j];
                break;
            case 2:
                testdata[i][j] = bBand[j];
                break;
            default:
                break;
            }
        }
    }

    int* label = new int[iScaleWidth * iScaleHeight];

    // 用欧氏距离对待分类样本进行判定
    for (int i = 0; i < iScaleWidth * iScaleHeight; i++)
    {
        double mindist = std::numeric_limits<double>::max();
        int temp = 0;
        for (int j = 0; j < centernum; j++)
        {
            double d = sqrt((testdata[0][i] - center[j][0])*(testdata[0][i] - center[j][0]) + (testdata[1][i] - center[j][1])*(testdata[1][i] - center[j][1])
                            + (testdata[2][i] - center[j][2])*(testdata[2][i] - center[j][2]));
            if ( d < mindist)
            {
                mindist = d;
                temp = j;
            }
        }
        label[i] = temp;
    }

    // 输出分类结果
    unsigned char* output = new unsigned char[iScaleWidth * iScaleHeight];

    for (int i = 0; i < iScaleWidth * iScaleHeight; i++)
    {
        if (label[i] == 0)
        {
            output[i] = 0;
        }
        if (label[i] == 1)
        {
            output[i] = 50;
        }
        if (label[i] == 2)
        {
            output[i] = 100;
        }
        if (label[i] == 3)
        {
            output[i] = 150;
        }
        if (label[i] == 4)
        {
            output[i] = 200;
        }
    }

    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = output[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = output[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = output[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);

    SaveImageWithGDAL(output,iScaleWidth,iScaleHeight);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}
#pragma endregion}

#pragma endregion}


void operation::NDVI()
{
    // 读取影像数据
    int w = rasterDataset -> GetRasterXSize();
    int h = rasterDataset -> GetRasterYSize();
    float * rBand = new float[iScaleWidth * iScaleHeight];
    float * NIRBand = new float[iScaleWidth * iScaleHeight];
    rasterDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, w, h, rBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);
    rasterDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, w, h, NIRBand, iScaleWidth , iScaleHeight, GDT_Float32, 0, 0);

    QVector<float> r;
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        r.append(rBand[i]);
    }
    QVector<float> nir;
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        nir.append(NIRBand[i]);
    }
    // 植被指数
    float *buffer = new float[iScaleWidth * iScaleHeight];
    for(int i = 0;i < iScaleWidth * iScaleHeight;i++)
    {
        buffer[i] = (int)((NIRBand[i] - rBand[i] ) / (rBand[i] + NIRBand[i]));
    }
    delete[] rBand;
    delete[] NIRBand;

    // 拉伸至0-255
    float min = 9999;
    float max0 = -1;
    for(int i = 0;i < iScaleHeight * iScaleWidth;i++)
    {
        if(buffer[i] < min)
            min = buffer[i];
        else if(buffer[i] > max0)
            max0 = buffer[i];
    }
    for (int i = 0; i < iScaleHeight * iScaleWidth; i++)
    {
        if (buffer[i] > max0)
        {
            buffer[i] = 255;
        }
        else if (buffer[i] <= max0&& buffer[i] >= min)
        {
            buffer[i] = static_cast<uchar>(255 - 255 * (max0 - buffer[i]) / (max0 - min));
        }
        else
        {
            buffer[i] = 0;
        }
    }

    QVector<QVector<float>> sigma;    // 类间方差
    sigma.resize(2);
    for(int k = 0;k < 255;k++)
    {
        // 前景和背景
        QVector<float> front;
        QVector<float> back;
        float sumfront = 0; // 前景的像素总数
        float sumback = 0;  // 背景的像素总数
        float meanfront = 0; // 前景的像素平均值
        float meanback = 0;  // 背景的像素平均值
        float pfront = 0;   // 前景的像素概率
        float pback = 0;    // 背景的像素概率
        for(int i = 0;i < iScaleHeight * iScaleWidth;i++)
        {
            if(buffer[i] > k)
            {
                sumfront += buffer[i];
                front.append(buffer[i]);
            }
            else
            {
                sumback += buffer[i];
                back.append(buffer[i]);
            }
        }
        meanfront = sumfront / front.length();
        meanback = sumback / back.length();
        pfront = sumfront / (iScaleHeight * iScaleWidth);
        pback = sumback / (iScaleHeight * iScaleWidth);

        sigma[0].append(1.0 * k);
        sigma[1].append(pfront * pback * qPow(meanfront - meanback,2));
    }

    float value = 0;
    float max = sigma[1][0];
    for(int i = 1;i < sigma[0].length();i++)
    {
        if(max < sigma[1][i])
        {
            max = sigma[1][i];
            value = sigma[0][i];
        }
    }

    QVector<float> newBand;
    for(int i =0;i < iScaleHeight * iScaleWidth;i++)
    {
        if(buffer[i] > (int)value)
            newBand.append(255);
        else
            newBand.append(0);
    }
    delete[] buffer;

    // 将三个波段组合起来
    int bytePerLine = (iScaleWidth * 24 + 31) / 8;
    unsigned char* allBandUC = new unsigned char[bytePerLine * iScaleHeight];
    for (int h = 0; h < iScaleHeight; h++)
    {
        for (int w = 0; w < iScaleWidth; w++)
        {
            allBandUC[h * bytePerLine + w * 3 + 0] = newBand[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 1] = newBand[h * iScaleWidth + w];
            allBandUC[h * bytePerLine + w * 3 + 2] = newBand[h * iScaleWidth + w];
        }
    }

    image=new QImage(allBandUC, iScaleWidth, iScaleHeight, bytePerLine, QImage::Format_RGB888);
    //SaveImageWithGDAL(allBandUC,iScaleWidth,iScaleHeight);

    // 构造图像并显示
    QGraphicsPixmapItem *imgItem = new QGraphicsPixmapItem(QPixmap::fromImage(*image));

    myscene = new QGraphicsScene();
    myscene->addItem(imgItem);
}



#pragma region"保存影像"{
void operation::SaveImageWithGDAL(unsigned char *allBandUC, int iScaleWidth, int iScaleHeight)
{
    const char *pszFormat = "GTiff";
    GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);

    if (poDriver == nullptr)
    {
        exit(1); // 如果没有找到PNG驱动则退出
    }

    char **papszMetadata = poDriver->GetMetadata();
    if (!CSLFetchBoolean(papszMetadata, GDAL_DCAP_CREATE, FALSE))
    {
        exit(1); // 如果驱动不支持创建新文件则退出
    }
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save Image", "", "TIFF Files (*.tif);;All Files (*)", new QString("TIFF Files (*.tif)"));

    // 创建一个新的数据集
    GDALDataset *poDstDS;
    char **papszOptions = NULL;
    poDstDS = poDriver->Create(fileName.toStdString().c_str(), iScaleWidth, iScaleHeight, 1, GDT_Byte, papszOptions);

    int bandSize = iScaleWidth * iScaleHeight;

    // 创建三个数组，每个数组对应一个波段
    unsigned char *band1 = new unsigned char[bandSize];

    // 从原始数组中拆分数据到三个波段
    for (int i = 0; i < bandSize; i++)
    {
        band1[i] = allBandUC[i];     // 第一个波段
    }

    // 写入数据
    poDstDS->GetRasterBand(1)->RasterIO(GF_Write,0,0,iScaleWidth,iScaleHeight,band1,iScaleWidth,iScaleHeight,GDT_Byte,0,0);

    // 关闭数据集
    delete[] band1;
    GDALClose((GDALDatasetH)poDstDS);
}
#pragma endregion}
