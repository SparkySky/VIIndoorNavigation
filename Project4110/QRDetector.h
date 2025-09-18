#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class QRDetector {
public:
    QRDetector();
    cv::Rect detect(cv::Mat&, std::string&);
    std::string detect(const cv::Mat& frame);
    cv::Mat cropped;
    cv::Mat upscaled;
private:
    cv::Mat grayscale, thresold;
    cv::Mat extractRedRegion(const cv::Mat& frame);
};