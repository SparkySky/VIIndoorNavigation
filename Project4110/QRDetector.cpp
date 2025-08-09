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




#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include "QRDetector.h"
#include "supp.h"

using namespace cv;
using namespace std;

extern Mat win[];

Mat QRDetector::extractRedRegion(const Mat& frame) {
    Mat hsv, mask1, mask2, mask;
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    // Red has two ranges in HSV
    //inRange(hsv, Scalar(8, 70, 50), Scalar(13, 255, 255), mask1);   // Lower red range
    //inRange(hsv, Scalar(167, 70, 50), Scalar(175, 255, 255), mask2); // Upper red range
    inRange(hsv, Scalar(8, 150, 100), Scalar(13, 255, 255), mask1);   // Lower red range
    inRange(hsv, Scalar(167, 150, 100), Scalar(180, 255, 255), mask2); // Upper red range

    mask = mask1 | mask2;
    
    return mask;
}

string QRDetector::detect(const Mat& frame) {
    Mat mask = extractRedRegion(frame);
    vector<vector<Point>> contours;

    Mat RGBmask;
    cvtColor(mask, RGBmask, COLOR_GRAY2BGR);
    RGBmask.copyTo(win[1]);

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

    //imshow("Color Processing and QR Detection", largeWin);
    return "";
}
