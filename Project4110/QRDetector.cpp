//#include "QRDetector.h"
//#include	<opencv2/core/core.hpp> // The core functions of opencv
//#include	<opencv2/highgui/highgui.hpp> // The GUI functions of opencv
//
//using namespace cv;
//
//cv::Mat QRDetector::extractRedRegion(const cv::Mat& frame) {
//    //cv::Mat hsv, mask1, mask2, mask;
//    //cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
//    //cv::inRange(hsv, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), mask1);
//    //cv::inRange(hsv, cv::Scalar(160, 100, 100), cv::Scalar(180, 255, 255), mask2);
//    //mask = mask1 | mask2;
//    //imshow(mask);
//    return frame;
//}
//
//std::string QRDetector::detect(const cv::Mat& frame) {
//    cv::Mat mask = extractRedRegion(frame);
//    std::vector<std::vector<cv::Point>> contours;
//    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
//
//    for (const auto& contour : contours) {
//        cv::Rect bbox = cv::boundingRect(contour);
//
//        // Ensure bbox is within frame bounds
//        bbox &= cv::Rect(0, 0, frame.cols, frame.rows);
//        if (bbox.width <= 0 || bbox.height <= 0) continue;
//
//        cv::Mat cropped = frame(bbox);
//        cv::QRCodeDetector qrDecoder;
//        std::string data = qrDecoder.detectAndDecode(cropped);
//        if (!data.empty()) return data;
//    }
//    return "";
//}

#include "QRDetector.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;

cv::Mat QRDetector::extractRedRegion(const cv::Mat& frame) {
    cv::Mat hsv, mask1, mask2, mask;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // Red has two ranges in HSV
    //cv::inRange(hsv, cv::Scalar(8, 70, 50), cv::Scalar(13, 255, 255), mask1);   // Lower red range
    //cv::inRange(hsv, cv::Scalar(167, 70, 50), cv::Scalar(175, 255, 255), mask2); // Upper red range
    cv::inRange(hsv, cv::Scalar(8, 150, 100), cv::Scalar(13, 255, 255), mask1);   // Lower red range
    cv::inRange(hsv, cv::Scalar(167, 150, 100), cv::Scalar(180, 255, 255), mask2); // Upper red range



    mask = mask1 | mask2;

    cv::imshow("Red Mask", mask);

    return mask;
}

std::string QRDetector::detect(const cv::Mat& frame) {
    cv::Mat mask = extractRedRegion(frame);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        cv::Rect bbox = cv::boundingRect(contour);
        bbox &= cv::Rect(0, 0, frame.cols, frame.rows);
        if (bbox.width <= 0 || bbox.height <= 0) continue;

        cv::Rect safeBox = bbox & cv::Rect(0, 0, frame.cols, frame.rows);
        if (safeBox.width > 0 && safeBox.height > 0) {
            cv::Mat cropped = frame(safeBox);
            if (!cropped.empty()) {
                cv::Mat upscaled;
                cv::resize(cropped, upscaled, cv::Size(), 2.0, 2.0, cv::INTER_LINEAR);

                cv::rectangle(frame, safeBox, cv::Scalar(0, 255, 0), 2);
                //cv::imshow("Detected Region", frame);

                cv::QRCodeDetector qrDecoder;
                std::string data = qrDecoder.detectAndDecode(upscaled);
                if (!data.empty()) return data;
            }
        }

    }

    return "";
}
