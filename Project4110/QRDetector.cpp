#define NOMINMAX

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <algorithm>

#include "QRDetector.h"

using namespace cv;
using namespace std;

boolean justLaunch = 1;
QRDetector::QRDetector() {}
extern Mat win[];

// Helper function to calculate squared distance between two points
// Using squared distance is often faster as it avoids the sqrt operation
double distSq(const cv::Point& p1, const cv::Point& p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

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
    try {
        decodedData = qrDecoder.detectAndDecode(frame);
    }
    catch (Exception e) {
        cout << "Unable to decode QR\n";
    }

    return bbox;
}


cv::Mat QRDetector::extractRedRegion(const cv::Mat& frame) {
    // Configure batch size here - change this value as needed
    const int kBatchSize = 20;

    // Configure confidence threshold (percentage) - only show results above this confidence
    const double kConfidenceThreshold = 10.0;

    // Enable/disable text-to-speech
    const bool kEnableTts = true;

    static int frameCounter = 0;
    static std::vector<std::string> detectionResults;
    static std::string currentBestResult = "No detection yet...";
    static int currentBestCount = 0;
    static std::string confirmedResultToSpeak = ""; // The result we have already announced
    static int stableFrameCount = 0;

    int screenWidth = frame.cols;
    cv::Mat lab, mask, bgr, outputFrame = frame.clone();

    // Step 1: Convert to CIELAB color space
    cv::cvtColor(frame, lab, cv::COLOR_BGR2Lab);

    // Step 2: Define red thresholds in LAB

    // Red typically has high 'a' (green-red) and low 'b' (blue-yellow)
    //cv::Scalar lower_red(20, 150, 130); // L, a, b
    //cv::Scalar upper_red(255, 255, 180);

    // Wider, more inclusive range. Focused on Recall
    //cv::Scalar lower_red(0, 140, 125);
    //cv::Scalar upper_red(255, 255, 185);

    // Manual Adjust
    cv::Scalar lower_red(10, 140, 125);
    cv::Scalar upper_red(255, 255, 185);


    // Step 3: Threshold to get red mask
    cv::inRange(lab, lower_red, upper_red, mask);

    // Step 4: Morphological cleanup
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    //// Step 5: Convert mask to BGR for visualization
    //cv::cvtColor(mask, bgr, cv::COLOR_GRAY2BGR);

    //// Step 6: Find contours
    //std::vector<std::vector<cv::Point>> contours;
    //cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    int leftCount = 0, centerCount = 0, rightCount = 0;
    cv::cvtColor(mask, bgr, cv::COLOR_GRAY2BGR); // Prepare BGR mask for drawing

    // Call the new module to do the actual shape detection and drawing
    shapeRecognition(mask, outputFrame, lab, bgr, leftCount, centerCount, rightCount);
    //cv::Mat outputFrame = frame.clone();
    //const double minArea = 300;

    //for (size_t i = 0; i < contours.size(); i++) {
    //    if (cv::contourArea(contours[i]) < minArea) {
    //        continue;
    //    }

    //    cv::RotatedRect rotatedRect = cv::minAreaRect(contours[i]);
    //    cv::Rect boundingBox = rotatedRect.boundingRect(); // Get the bounding box from the rotated rect

    //    float width = rotatedRect.size.width;
    //    float height = rotatedRect.size.height;
    //    float aspectRatio = std::max(width, height) / std::min(width, height);

    //    std::vector<cv::Point> approx;
    //    double epsilon = 0.04 * cv::arcLength(contours[i], true);
    //    cv::approxPolyDP(contours[i], approx, epsilon, true);

    //    bool isValidRegion = false;
    //    if (approx.size() == 4 && aspectRatio < 1.3) {
    //        isValidRegion = true;
    //    }

    //    if (isValidRegion) {
    //        cv::rectangle(outputFrame, boundingBox, cv::Scalar(0, 255, 0), 2);
    //        cv::rectangle(lab, boundingBox, cv::Scalar(0, 255, 0), 2);
    //        cv::rectangle(bgr, boundingBox, cv::Scalar(0, 255, 0), 2);

    //        int centerX = boundingBox.x + boundingBox.width / 2;

    //        if (centerX < screenWidth / 3) {
    //            leftCount++;
    //        }
    //        else if (centerX < 2 * screenWidth / 3) {
    //            centerCount++;
    //        }
    //        else {
    //            rightCount++;
    //        }
    //    }
    //}

    // Generate current frame detection message
    std::string currentFrameResult;
    if (leftCount == 0 && centerCount == 0 && rightCount == 0) {
        currentFrameResult = "No valid red regions detected.";
    }
    else {
        currentFrameResult = "Detected: ";
        if (leftCount > 0) {
            currentFrameResult += "Left (" + std::to_string(leftCount) + ")";
        }
        if (centerCount > 0) {
            if (leftCount > 0) currentFrameResult += ", ";
            currentFrameResult += "Center (" + std::to_string(centerCount) + ")";
        }
        if (rightCount > 0) {
            if (leftCount > 0 || centerCount > 0) currentFrameResult += ", ";
            currentFrameResult += "Right (" + std::to_string(rightCount) + ")";
        }
    }

    // Store the detection result
    detectionResults.push_back(currentFrameResult);
    frameCounter++;

    // Update running statistics
    if (frameCounter >= 10) { // Start showing stats after 10 frames
        std::map<std::string, int> resultCount;
        for (const auto& result : detectionResults) {
            resultCount[result]++;
        }

        int maxCount = 0;
        std::string bestResult = "Low confidence";
        std::string previousBestResult = currentBestResult;

        for (const auto& pair : resultCount) {
            if (pair.second > maxCount) {
                maxCount = pair.second;
                double confidence = (100.0 * maxCount / frameCounter);

                // Only accept result if it meets confidence threshold
                if (confidence >= kConfidenceThreshold) {
                    bestResult = pair.first;
                }
                else {
                    bestResult = "Low confidence (" + std::to_string((int)confidence) + "%)";
                }
            }
        }

        const int kStabilityThreshold = 10; // Require 15 stable frames before speaking (about 0.5s)

        // Check if the best result has changed from the previous frame
        if (bestResult == previousBestResult) {
            stableFrameCount++; // If it's the same, it's more stable
        }
        else {
            stableFrameCount = 0; // If it changed, reset the stability counter
        }

        // Check if best result changed and trigger TTS
        if (kEnableTts &&
            bestResult != "Low confidence" &&
            stableFrameCount > kStabilityThreshold &&
            bestResult != confirmedResultToSpeak)
        {
            std::string guidanceMessage;

            // Create speech-friendly text
            if (bestResult.find("No valid red regions") != std::string::npos) {
                if (justLaunch) {
                    justLaunch = 0;
                }
                else {
                    guidanceMessage = "Potential QR no longer detected";
                }
            }
            else if (bestResult.find("Detected:") != std::string::npos) {
                // Example: "Detected: Left (1), Center (2), Right (1)"
                std::string detected = bestResult.substr(10); // remove "Detected: "
                std::vector<std::string> parts;
                std::stringstream ss(detected);
                std::string item;

                while (std::getline(ss, item, ',')) {
                    parts.push_back(item);
                }

                std::string combined;
                for (size_t i = 0; i < parts.size(); i++) {
                    std::string part = parts[i];
                    std::string messagePart;

                    bool centerOnly = 1;
                    if (part.find("Left") != std::string::npos) {
                        messagePart = "on the left";
                        centerOnly = 0;
                    }
                    else if (part.find("Center") != std::string::npos) {
                        messagePart = "in the center";
                    }
                    else if (part.find("Right") != std::string::npos) {
                        messagePart = "on the right";
                        centerOnly = 0;
                    }

                    // Extract number inside ()
                    size_t start = part.find("(");
                    size_t end = part.find(")");
                    std::string number = "1";
                    if (start != std::string::npos && end != std::string::npos) {
                        number = part.substr(start + 1, end - start - 1);
                    }

                    combined += number + " QR code" + (number == "1" ? "" : "s") + " " + messagePart;
                    if (i < parts.size() - 1) {
                        combined += " and ";
                    }
                    if (centerOnly)
                        combined += ". Please move forward.";
                }

                guidanceMessage = "Potential " + combined;
            }

            // Use Text2Speech library
            narrate.speak_low_priority(guidanceMessage);
            std::cout << guidanceMessage << std::endl;
            confirmedResultToSpeak = bestResult;
        }
        currentBestResult = bestResult;
        currentBestCount = maxCount;
    }

    // Add frame divisions and labels
    int leftSideWidth = screenWidth / 3;
    int rightSideWidth = screenWidth * 2 / 3;
    cv::line(outputFrame, { leftSideWidth, 0 }, { leftSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::line(outputFrame, { rightSideWidth, 0 }, { rightSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::putText(outputFrame, "Left", { leftSideWidth / 2 - 20, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(outputFrame, "Center", { leftSideWidth + screenWidth / 11, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(outputFrame, "Right", { rightSideWidth + leftSideWidth / 3, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);

    cv::line(lab, { leftSideWidth, 0 }, { leftSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::line(lab, { rightSideWidth, 0 }, { rightSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::putText(lab, "Left", { leftSideWidth / 2 - 20, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(lab, "Center", { leftSideWidth + screenWidth / 11, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(lab, "Right", { rightSideWidth + leftSideWidth / 3, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);

    cv::line(bgr, { leftSideWidth, 0 }, { leftSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::line(bgr, { rightSideWidth, 0 }, { rightSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::putText(bgr, "Left", { leftSideWidth / 2 - 20, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(bgr, "Center", { leftSideWidth + screenWidth / 11, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(bgr, "Right", { rightSideWidth + leftSideWidth / 3, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);

    // Always show current frame result + running best result
    cv::putText(outputFrame, "Current: " + currentFrameResult, { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, { 255, 255, 0 }, 2);
    cv::putText(outputFrame, "Best (" + std::to_string(currentBestCount) + "/" + std::to_string(frameCounter) + "): " + currentBestResult, { 10, 60 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, { 0, 0, 255 }, 2);

    cv::putText(lab, "Current: " + currentFrameResult, { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, { 255, 255, 0 }, 2);
    cv::putText(lab, "Best (" + std::to_string(currentBestCount) + "/" + std::to_string(frameCounter) + "): " + currentBestResult, { 10, 60 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, { 0, 0, 255 }, 2);

    cv::putText(bgr, "Current: " + currentFrameResult, { 10, 30 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, { 255, 255, 0 }, 2);
    cv::putText(bgr, "Best (" + std::to_string(currentBestCount) + "/" + std::to_string(frameCounter) + "): " + currentBestResult, { 10, 60 }, cv::FONT_HERSHEY_SIMPLEX, 0.6, { 0, 0, 255 }, 2);


    // Keep only last kBatchSize results to maintain sliding window
    if (detectionResults.size() > kBatchSize) {
        detectionResults.erase(detectionResults.begin());
        frameCounter = kBatchSize; // Keep counter at kBatchSize for sliding window
    }

    outputFrame.copyTo(win[1]);
    lab.copyTo(win[2]);
    bgr.copyTo(win[3]);

    return mask;
}


// dedicated function for shape analysis.
void QRDetector::shapeRecognition(
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

                            imshow("QR Cropped", cropped);
                            waitKey(1);

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