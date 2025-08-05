#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class QRDetector {
public:
    std::string detect(const cv::Mat& frame);
private:
    cv::Mat extractRedRegion(const cv::Mat& frame);
};