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

//#include "Source.cpp"
#include "QRDetector.h"
#include "supp.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;
extern Mat win[];

cv::Mat QRDetector::extractRedRegion(const cv::Mat& frame) {
    cv::Mat hsv, mask1, mask2, mask;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // Red has two ranges in HSV
    //cv::inRange(hsv, cv::Scalar(8, 70, 50), cv::Scalar(13, 255, 255), mask1);   // Lower red range
    //cv::inRange(hsv, cv::Scalar(167, 70, 50), cv::Scalar(175, 255, 255), mask2); // Upper red range
    cv::inRange(hsv, cv::Scalar(8, 150, 100), cv::Scalar(13, 255, 255), mask1);   // Lower red range
    cv::inRange(hsv, cv::Scalar(167, 150, 100), cv::Scalar(180, 255, 255), mask2); // Upper red range



    mask = mask1 | mask2;
    
    return mask;
}

std::string QRDetector::detect(const cv::Mat& frame) {
    cv::Mat mask = extractRedRegion(frame);
    std::vector<std::vector<cv::Point>> contours;

    Mat RGBmask;
    cv::cvtColor(mask, RGBmask, cv::COLOR_GRAY2BGR);
    RGBmask.copyTo(win[1]);

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
                imshow("QR Cropped", cropped);
                imshow("QR Upscaled", upscaled);
                cv::QRCodeDetector qrDecoder;
                
                if (!upscaled.empty() && upscaled.cols > 0 && upscaled.rows > 0) {
                    std::string data = qrDecoder.detectAndDecode(upscaled);
                    if (!data.empty())
                        cout << "found\n";
                        return data;
                }

            }
        }
    }

    //imshow("Color Processing and QR Detection", largeWin);
    return "";
}
