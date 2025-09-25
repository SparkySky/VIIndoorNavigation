#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class ShapeAnalysis {
public:
    ShapeAnalysis();
    void shapeRecognition(
        const cv::Mat& mask,         // INPUT: The binary mask from color processing
        cv::Mat& outputFrame,        // OUTPUT: The frame to draw results on
        cv::Mat& lab,                // OUTPUT: Debug frame
        cv::Mat& bgr,                // OUTPUT: Debug frame
        int& leftCount,              // OUTPUT: Count of shapes on the left
        int& centerCount,            // OUTPUT: Count of shapes in the center
        int& rightCount);            // OUTPUT: Count of shapes on the right
    int minAreaTrackbar = 900;
private:

};