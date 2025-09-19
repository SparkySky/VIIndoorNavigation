#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "Text2Speech.h"

class QRDetector {
public:
    std::string detect(const cv::Mat& frame);
    cv::Mat cropped;
    cv::Mat upscaled;
private:
    cv::Mat extractRedRegion(const cv::Mat& frame);
    Text2Speech narrate;
};