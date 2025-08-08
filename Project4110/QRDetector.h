#pragma once
#include <opencv2/opencv.hpp>
#include <string>

using namespace std;
using namespace cv;

class QRDetector {
public:
    string detect(const Mat& frame);
    Mat cropped;
    Mat upscaled;
private:
    Mat extractRedRegion(const Mat& frame);
};