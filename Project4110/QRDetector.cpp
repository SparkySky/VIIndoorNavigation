#define NOMINMAX

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <algorithm>

#include "QRDetector.h"
#include "UIForVI.h"
#include "ShapeAnalysis.h"

using namespace cv;
using namespace std;

QRDetector::QRDetector() {}
extern Mat win[];
UIForVI ui;

cv::Rect QRDetector::detect(cv::Mat& frame, std::string& decodedData, UIForVI& uiManager, ShapeAnalysis& analyser) {
    cv::Mat redRegion = colourProcessing.extractRedRegion(frame, analyser);
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
    try {
        decodedData = qrDecoder.detectAndDecode(frame);
    }
    catch (Exception e) {
        cout << "Unable to decode QR\n";
    }

    return bbox;
}

string QRDetector::detect(const Mat& frame, UIForVI& uiManager, ShapeAnalysis& analyser) {
    Mat mask = colourProcessing.extractRedRegion(frame, analyser);
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
                            imshow("QR Cropped", cropped);
                            waitKey(1);
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