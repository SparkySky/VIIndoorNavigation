#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include "QRDetector.h"

using namespace cv;
using namespace std;

extern Mat win[];

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
                    string data = qrDecoder.detectAndDecode(upscaled);
                    if (!data.empty()) {
                        cout << "Found\n";
                        imshow("QR Cropped", cropped);
                        return data;
                    }
                }
            }
        }
    }
    return "";
}

// Old 
//#include "QRDetector.h"
//#include	<opencv2/core/core.hpp> // The core functions of opencv
//#include	<opencv2/highgui/highgui.hpp> // The GUI functions of opencv
//
//using namespace cv;
//
//Mat QRDetector::extractRedRegion(const Mat& frame) {
//    //Mat hsv, mask1, mask2, mask;
//    //cvtColor(frame, hsv, COLOR_BGR2HSV);
//    //inRange(hsv, Scalar(0, 100, 100), Scalar(10, 255, 255), mask1);
//    //inRange(hsv, Scalar(160, 100, 100), Scalar(180, 255, 255), mask2);
//    //mask = mask1 | mask2;
//    //imshow(mask);
//    return frame;
//}
//
//string QRDetector::detect(const Mat& frame) {
//    Mat mask = extractRedRegion(frame);
//    vector<vector<Point>> contours;
//    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
//
//    for (const auto& contour : contours) {
//        Rect bbox = boundingRect(contour);
//
//        // Ensure bbox is within frame bounds
//        bbox &= Rect(0, 0, frame.cols, frame.rows);
//        if (bbox.width <= 0 || bbox.height <= 0) continue;
//
//        Mat cropped = frame(bbox);
//        QRCodeDetector qrDecoder;
//        string data = qrDecoder.detectAndDecode(cropped);
//        if (!data.empty()) return data;
//    }
//    return "";
//}


// For testing
//cv::Mat QRDetector::extractRedRegion(const cv::Mat& frame) {
//    cv::Mat hsv, mask1, mask2, mask, segmented;
//
//    // --- One-time UI setup with trackbars ---
//    static bool uiInit = false;
//
//    // Sliders for lower red band (near Hue ~0)
//    static int h_low1 = 8, s_low1 = 150, v_low1 = 100;
//    static int h_high1 = 13, s_high1 = 255, v_high1 = 255;
//
//    // Sliders for upper red band (near Hue ~180)
//    static int h_low2 = 167, s_low2 = 150, v_low2 = 100;
//    static int h_high2 = 180, s_high2 = 255, v_high2 = 255;
//
//    // Cleanup slider
//    static int morph_ksize = 3; // odd 1..21
//
//    if (!uiInit) {
//        uiInit = true;
//        cv::namedWindow("HSV Image", cv::WINDOW_NORMAL);
//        cv::namedWindow("Red Mask", cv::WINDOW_NORMAL);
//        cv::namedWindow("Segmented", cv::WINDOW_NORMAL);
//        cv::resizeWindow("HSV Image", 480, 360);
//        cv::resizeWindow("Red Mask", 480, 360);
//        cv::resizeWindow("Segmented", 480, 360);
//
//        // Lower band trackbars
//        cv::createTrackbar("h_low1", "Red Mask", &h_low1, 180);
//        cv::createTrackbar("h_high1", "Red Mask", &h_high1, 180);
//        cv::createTrackbar("s_low1", "Red Mask", &s_low1, 255);
//        cv::createTrackbar("s_high1", "Red Mask", &s_high1, 255);
//        cv::createTrackbar("v_low1", "Red Mask", &v_low1, 255);
//        cv::createTrackbar("v_high1", "Red Mask", &v_high1, 255);
//
//        // Upper band trackbars
//        cv::createTrackbar("h_low2", "Red Mask", &h_low2, 180);
//        cv::createTrackbar("h_high2", "Red Mask", &h_high2, 180);
//        cv::createTrackbar("s_low2", "Red Mask", &s_low2, 255);
//        cv::createTrackbar("s_high2", "Red Mask", &s_high2, 255);
//        cv::createTrackbar("v_low2", "Red Mask", &v_low2, 255);
//        cv::createTrackbar("v_high2", "Red Mask", &v_high2, 255);
//
//        // Morphology kernel (odd size)
//        cv::createTrackbar("morph ksize (odd)", "Red Mask", &morph_ksize, 21);
//    }
//
//    // --- Processing ---
//    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
//
//    // Clamp & order hue bounds just in case sliders cross
//    if (h_low1 > h_high1)  std::swap(h_low1, h_high1);
//    if (h_low2 > h_high2)  std::swap(h_low2, h_high2);
//    if (s_low1 > s_high1)  std::swap(s_low1, s_high1);
//    if (s_low2 > s_high2)  std::swap(s_low2, s_high2);
//    if (v_low1 > v_high1)  std::swap(v_low1, v_high1);
//    if (v_low2 > v_high2)  std::swap(v_low2, v_high2);
//
//    // Two red bands
//    cv::inRange(hsv, cv::Scalar(h_low1, s_low1, v_low1),
//        cv::Scalar(h_high1, s_high1, v_high1), mask1);
//
//    cv::inRange(hsv, cv::Scalar(h_low2, s_low2, v_low2),
//        cv::Scalar(h_high2, s_high2, v_high2), mask2);
//
//    // Merge
//    cv::bitwise_or(mask1, mask2, mask);
//
//    // Morphological cleanup
//    int k = std::max(1, morph_ksize | 1); // force odd, >=1
//    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(k, k));
//    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
//
//    // Segmented preview
//    cv::bitwise_and(frame, frame, segmented, mask);
//
//    // --- Show windows ---
//    cv::imshow("HSV Image", hsv);
//    cv::imshow("Red Mask", mask);
//    cv::imshow("Segmented", segmented);
//
//    return mask; // This is the final binary mask for red regions
//}

//Mat QRDetector::extractRedRegion(const Mat& frame) {
//    Mat hsv, mask1, mask2, mask;
//    cvtColor(frame, hsv, COLOR_BGR2HSV);
//
//    // Red has two ranges in HSV
//    //inRange(hsv, Scalar(8, 70, 50), Scalar(13, 255, 255), mask1);   // Lower red range
//    //inRange(hsv, Scalar(167, 70, 50), Scalar(175, 255, 255), mask2); // Upper red range
//    inRange(hsv, Scalar(8, 150, 100), Scalar(13, 255, 255), mask1);   // Lower red range
//    inRange(hsv, Scalar(167, 150, 100), Scalar(180, 255, 255), mask2); // Upper red range
//
//    mask = mask1 | mask2;
//    
//    return mask;
//}