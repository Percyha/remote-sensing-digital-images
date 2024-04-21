#ifndef OPERATION_H
#define OPERATION_H

#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QBarSet>
#include <QBarSeries>
#include <QChart>
#include <QValueAxis>
#include <QVector>
#include <QLogValueAxis>
#include <QChartView>
#include <QColor>
#include <QLineSeries>
#include <QSplineSeries>
#include <QScatterSeries>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsScene>

#include <qalgorithms.h>
#include <QtAlgorithms>
#include <QtMath>

#include <Eigen/Dense>
#include <opencv2/core/eigen.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d/xfeatures2d.hpp>
#include <opencv2/nonfree.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <stdlib.h>

#include <gdal_priv.h>
#include <gdalwarper.h>
#include <cpl_conv.h>

#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_widget.h>
#include<qwt_scale_div.h>

#include <QDebug>
#include <QLibrary>
#include <QFile>

using namespace cv;
using namespace cv::xfeatures2d;
using namespace Eigen;

class operation
{
public:
    operation();

    const double a = 6378245.0;
    const double bb = 6356863.0187730473;
    double e = qSqrt(a * a - bb * bb) / a;
    double ee = qSqrt(a * a - bb * bb) / bb;
    const double p = 180 * 60 * 60 / 3.1415926535;
    const double m0 = 6335552.71700;
    const double m2 = 63609.78833;
    const double m4 = 532.20892;
    const double m6 = 4.15602;
    const double m8 = 0.03130;
    const double a0 = m0 + m2 / 2 + 3 * m4 / 8 + 5 * m6 / 16 + 35 * m8 / 128;
    const double a2 = m2 / 2 + m4 / 2 + 15 * m6 / 32 + 7 * m8 / 16;
    const double a4 = m4 / 8 + 3 * m6 / 16 + 7 * m8 / 32;
    const double a6 = m6 / 32 + m8 / 16;
    const double a8 = m8 / 128;

    int currentIndex=-1; // 当前选择查看波段的波段号，第一个波段还是第二个波段还是第三个波段还是全波段

    QwtPlot *p1; // 绘制直方图

    int n=0; // 用于SIFT算法里
    Mat image1;
    Mat image2;
    std::vector<cv::KeyPoint> image1_kp;
    std::vector<cv::KeyPoint> image2_kp;

    int filterNum; // 滤波器的选择
    int dftnum; // 傅里叶变换选择
    int imgWidth;
    int imgHeight;
    float m_scaleFactor;
    int iScaleWidth;
    int iScaleHeight;
    bool show; // 是否显示直方图
    bool histormEqualization; // 是否要均衡化
    bool radiationCalibration; // 是否要辐射校正
    bool dixingjiaozheng; // 是否要地形校正
    bool lvboo;
    bool dft; // 是否进行傅里叶（DFT）变换

    int liner; // 线性拉伸的数值 2为2%拉伸

    QVector<QString> RasterInf;

    GDALDataset *rasterDataset;
    QwtPlotZoomer *m_zoomer;
    QwtPlotPanner *m_panner;
    QImage *image;
    QGraphicsScene * myscene;

    // 影像融合需要两个GDALDataset
    GDALDataset *highRaster;
    GDALDataset *lowRaster;

    // 打开遥感影像
    GDALDataset* OpenRasterFile();
    // 统计遥感影像信息
    QVector<QString> ShowRasterInformation(GDALDataset *poDataset);
    // 统计直方图信息
    void Historm(double height);
    void Historm(); // 绘制直方图
    QVector<float> GetHistormMAXMin(float *band,int total); // 统计波段的最大、最小灰度值
    QVector<float> GetAverageSteddv(float *band,int total);
    QVector<QVector<float>> SataticsBandsValue(float *band,int total); // 统计每个波段各个灰度值的个数
    QwtPlot * ShowHistorm(QVector<QVector<float>> satatics);
    QVector<QVector<float>> PaiXu(QVector<QVector<float>> bandHistorm);
    QVector<QVector<float>> Satatistics_Bands(float * rBand,float * gBand,float * bBand);
    // 显示遥感影像
    unsigned char * ImgSketch(float *buffer, int bandSize); // 最大最小拉伸
    QGraphicsScene * ShowRaster(float* r,float* g,float* b);
    // 2%线性拉伸
    unsigned char * ImgSketchPercent_2(float *buffer,int bandSize, double noValue); // 2%线性拉伸
    // 直方图均衡化
    unsigned char * HistormEqualization(float *buffer,int bandSize);
    // 辐射定标
    unsigned char* RadiationCalibration(float *buffer, int bandSize);
    // 大气校正
    void AtmosphericCorrection();
    // 卷积运算-均值滤波
    unsigned char * lvbo(float *buffer,int width,int height,int bandsize);
    // 把灰度值均缩放到0-255之间
    float *Stretch255(float *buffer,GDALRasterBand *currentBand,int bandSize);
    // 傅里叶变换(自己写的)
    void fuliye(float *buffer,int width,int height,int bandsize);
    // 傅里叶变换（opencv库）
    unsigned char* DFT(float *buffer,int row,int col);
    // 几何粗较正
    void ImageRegistration(float b,float l,float z,float Phi,float Kappa,float Omega,float Camera_f,float Camera_w,float Camera_h,float outSize);
    QVector<double> toXY(double B,double L); // 经纬度转地面高斯坐标
    double r(double x);
    double calm(double x);
    double caln(double x);
    double calX(double b); // 计算大地线长
    double calL(double L); // 计算中央子午线
    double d2h(double d);
    // 影像手动配准
    QVector<float> duoxiangshi(QVector<QPointF> Piexlxy,QVector<QPointF> GeoXY);
    void handpeizhun(QVector<int> src_x,QVector<int>src_y,QVector<int>ref_x,QVector<int>ref_y,QVector<int> psrc_x,QVector<int>psrc_y,QVector<int>pref_x,QVector<int>pref_y,QVector<double> geo);
    // 自动配准
    QGraphicsScene * SIFT();
    QGraphicsScene *SIFT_FLANN();
    // Brovey融合
    void Brovey();
    // IHS融合 + 直方图匹配
    void IHS();
    QVector<QVector<float>> HistogramMatching(float *buffer,int bandSize);
    // 定量评价
    float QuantitativeEvaluation(float *bufferR,float *bufferG,float *bufferB,int w,int h);
    // PCA变换融合
    void PCA();
    // 非监督训练之Kmeans
    void KMEANS();
    // 非监督训练之ISODATA
    void ISODATA();
    QVector<QVector<float>> julei(float *buffer,int k,QVector<float> label);
    QVector<QVector<float>> julei0(float *buffer,int k,QVector<float> label);
    int fenlie(QVector<QVector<float>> result,QVector<float> label,float s,int k);
    int hebing(QVector<QVector<float>> result,QVector<float> label,float c,int k);
    // 混合高斯模型
    double gauss(const double x, const double m, const double sigma);
    void GaussianMixtureModel();
    // 特征提取-角点检测之Harris
    void HARRIS();
    // 特征提取-角点检测之Moravec
    void MORAVEC();
    Mat MoravecCorners(Mat SrcImage, int kSize, int threshold);
    // 特征提取-直线检测
    void LineDetection();
    // 影像分割-迭代阈值分割
    void IterativeThresholdSegmentation();
    // 影像分割-OSTU（大津法）
    void OSTU();
    // 监督分类-最大似然法
    void MLC();
    // 监督分类-最小距离法
    void MinDistance();
    // 植被指数计算
    void NDVI();
    // 输出影像
    void SaveImageWithGDAL(unsigned char* allBandUC, int iScaleWidth, int iScaleHeight);

};

#endif // OPERATION_H
