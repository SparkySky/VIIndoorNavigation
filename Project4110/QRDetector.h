#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "Text2Speech.h"

class QRDetector {
public:
    QRDetector();
    cv::Rect detect(cv::Mat&, std::string&);
    std::string detect(const cv::Mat& frame);
    cv::Mat cropped;
    cv::Mat upscaled;
    int minAreaTrackbar = 300;
private:
    cv::Mat grayscale, thresold;
    cv::Mat extractRedRegion(const cv::Mat& frame);
    void shapeRecognition(
        const cv::Mat& mask,         // INPUT: The binary mask from color processing
        cv::Mat& outputFrame,        // OUTPUT: The frame to draw results on
        cv::Mat& lab,                // OUTPUT: Debug frame
        cv::Mat& bgr,                // OUTPUT: Debug frame
        int& leftCount,              // OUTPUT: Count of shapes on the left
        int& centerCount,            // OUTPUT: Count of shapes in the center
        int& rightCount);             // OUTPUT: Count of shapes on the right
    Text2Speech narrate;
};