#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include "Text2Speech.h"
#include "ShapeAnalysis.h"

class ColourProcessing {
public:
    ColourProcessing();
    cv::Mat extractRedRegion(const cv::Mat& frame, ShapeAnalysis& analyser);
private:
    Text2Speech narrate;
};