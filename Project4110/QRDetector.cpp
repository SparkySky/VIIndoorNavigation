#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <algorithm>

#include "QRDetector.h"

using namespace cv;
using namespace std;

QRDetector::QRDetector() {}
extern Mat win[];

cv::Rect QRDetector::detect(cv::Mat& frame, std::string& decodedData) {
    cv::Mat redRegion = extractRedRegion(frame);
    if (redRegion.empty()) {
        return cv::Rect();
    }

    std::vector<cv::Point> points;
    cv::findNonZero(redRegion, points);

    if (points.empty()) {
        return cv::Rect();
    }

    cv::Rect bbox = cv::boundingRect(points);

    cv::QRCodeDetector qrDecoder;
    decodedData = qrDecoder.detectAndDecode(frame);

    return bbox;
}


cv::Mat QRDetector::extractRedRegion(const cv::Mat& frame) {
    cv::Mat hsv, mask1, mask2, mask, segmented;

    // Fixed HSV thresholds for red detection
    // Lower red range (~0 hue)
    const int h_low1 = 0, s_low1 = 150, v_low1 = 100;
    const int h_high1 = 15, s_high1 = 255, v_high1 = 255;

    // Upper red range (~180 hue)
    const int h_low2 = 167, s_low2 = 150, v_low2 = 100;
    const int h_high2 = 180, s_high2 = 255, v_high2 = 255;

    // Morphology kernel size (odd number)
    const int morph_ksize = 3;

    // Convert to HSV
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    // First red band
    cv::inRange(hsv, cv::Scalar(h_low1, s_low1, v_low1),
        cv::Scalar(h_high1, s_high1, v_high1), mask1);

    // Second red band
    cv::inRange(hsv, cv::Scalar(h_low2, s_low2, v_low2),
        cv::Scalar(h_high2, s_high2, v_high2), mask2);

    // Merge the two masks
    cv::bitwise_or(mask1, mask2, mask);

    // Morphological cleanup
    cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_RECT, cv::Size(morph_ksize, morph_ksize)
    );
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

    // Segmented image (for debugging/preview)
    cv::bitwise_and(frame, frame, segmented, mask);

    // Optional display (remove for final release)
    //cv::imshow("HSV Image", hsv);
    //cv::imshow("Red Mask", mask);
    //cv::imshow("Segmented", segmented);
    hsv.copyTo(win[1]);
    cv::Mat mask_bgr;
    cv::cvtColor(mask, mask_bgr, cv::COLOR_GRAY2BGR);
    mask_bgr.copyTo(win[2]);
    segmented.copyTo(win[3]);

    return mask; // Binary mask for red regions
}


string QRDetector::detect(const Mat& frame) {
    Mat mask = extractRedRegion(frame);
    vector<vector<Point>> contours;

    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        Rect bbox = boundingRect(contour);
        bbox &= Rect(0, 0, frame.cols, frame.rows);
        if (bbox.width <= 0 || bbox.height <= 0) continue;

        Rect safeBox = bbox & Rect(0, 0, frame.cols, frame.rows);
        if (safeBox.width > 0 && safeBox.height > 0) {
            cropped = frame(safeBox);
            
            if (!cropped.empty()) {
                resize(cropped, upscaled, Size(), 2.0, 2.0, INTER_LINEAR);
                QRCodeDetector qrDecoder;
                
                if (!upscaled.empty() && upscaled.cols > 0 && upscaled.rows > 0) {
                    try {
                        string data = qrDecoder.detectAndDecode(upscaled);                    
                        if (!data.empty()) {
                            cout << "At node      : " << data << endl; // For Debug Nodes
                            //imshow("QR Cropped", cropped);
                            return data;
                        }
                    }
                    catch (exception e) {
                        return "";
                    };

                }
            }
        }
    }
    return "";
}