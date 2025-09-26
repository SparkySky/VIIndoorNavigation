// Yee Jia Hao
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <algorithm>

#include "ShapeAnalysis.h"

using namespace cv;
using namespace std;

ShapeAnalysis::ShapeAnalysis() {}

// Helper function to calculate squared distance between two points
// Using squared distance is often faster as it avoids the sqrt operation
double distSq(const cv::Point& p1, const cv::Point& p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

// dedicated function for shape analysis.
void ShapeAnalysis::shapeRecognition(
    const cv::Mat& mask,
    cv::Mat& outputFrame,
    cv::Mat& lab,
    cv::Mat& bgr,
    int& leftCount,
    int& centerCount,
    int& rightCount)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    int screenWidth = outputFrame.cols;

    for (const auto& contour : contours) {
        if (cv::contourArea(contour) < this->minAreaTrackbar) {
            continue;
        }

        std::vector<cv::Point> approx;
        double epsilon = 0.04 * cv::arcLength(contour, true);
        cv::approxPolyDP(contour, approx, epsilon, true);

        // SHAPE CHECK
        bool isValidShape = false;
        if (approx.size() == 4 && cv::isContourConvex(approx)) {
            // Get the 4 corner points
            cv::Point p1 = approx[0];
            cv::Point p2 = approx[1];
            cv::Point p3 = approx[2];
            cv::Point p4 = approx[3];

            // Calculate the squared lengths of the four sides
            double side1 = distSq(p1, p2);
            double side2 = distSq(p2, p3);
            double side3 = distSq(p3, p4);
            double side4 = distSq(p4, p1);

            // Find the shortest and longest side
            double minSideSq = std::min({ side1, side2, side3, side4 });
            double maxSideSq = std::max({ side1, side2, side3, side4 });

            // Check if the longest side is not too much longer than the shortest side
            // This ratio check is robust to perspective skew
            double sideRatio = maxSideSq / minSideSq;

            // Adjust this tolerance. Higher mean more lenient to shape(value: 2.0).
            if (sideRatio < 7.0) {
                isValidShape = true;
            }
        }

        if (isValidShape) {
            cv::Rect boundingBox = cv::boundingRect(approx); // Use the approx points for the bounding box

            // Draw and count the valid shape
            cv::rectangle(outputFrame, boundingBox, cv::Scalar(0, 255, 0), 2);
            cv::rectangle(lab, boundingBox, cv::Scalar(0, 255, 0), 2);
            cv::rectangle(bgr, boundingBox, cv::Scalar(0, 255, 0), 2);

            int centerX = boundingBox.x + boundingBox.width / 2;
            if (centerX < screenWidth / 3) {
                leftCount++;
            }
            else if (centerX < 2 * screenWidth / 3) {
                centerCount++;
            }
            else {
                rightCount++;
            }
        }
    }
}