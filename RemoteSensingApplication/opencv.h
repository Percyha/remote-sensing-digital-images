#ifndef OPENCV_H
#define OPENCV_H

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d/xfeatures2d.hpp>
using namespace cv;
class opencv
{
public:
    opencv();

    const int MAX_FEATURES = 500;
    const float GOOD_MATCH_PERCENT = 0.15f;

    void alignImages(Mat &im1, Mat &im2, Mat &im1Reg, Mat &h);
};

#endif // OPENCV_H
