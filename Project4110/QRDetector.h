#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "Text2Speech.h"
#include "UIForVi.h"
#include "ShapeAnalysis.h"

class QRDetector {
public:
    QRDetector();
    cv::Rect detect(cv::Mat&, std::string&, UIForVI&, ShapeAnalysis& analyser);
    std::string detect(const cv::Mat& frame, UIForVI&, ShapeAnalysis& analyser);
    cv::Mat cropped;
    cv::Mat upscaled;
    cv::Mat extractRedRegion(const cv::Mat& frame, ShapeAnalysis& analyser);
private:
    cv::Mat grayscale, thresold;
    Text2Speech narrate;
};