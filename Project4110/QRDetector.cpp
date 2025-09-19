#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include "QRDetector.h"

using namespace cv;
using namespace std;

extern Mat win[];

cv::Mat QRDetector::extractRedRegion(const cv::Mat& frame) {
    // Configure batch size here - change this value as needed
    const int kBatchSize = 20;

    // Configure confidence threshold (percentage) - only show results above this confidence
    const double kConfidenceThreshold = 40.0;

    // Enable/disable text-to-speech
    const bool kEnableTts = true;

    static int frameCounter = 0;
    static std::vector<std::string> detectionResults;
    static std::string currentBestResult = "No detection yet...";
    static int currentBestCount = 0;

    int screenWidth = frame.cols;



    cv::Mat lab, mask, bgr;

    // Step 1: Convert to CIELAB color space
    cv::cvtColor(frame, lab, cv::COLOR_BGR2Lab);

    // Step 2: Define red thresholds in LAB
    // Red typically has high 'a' (green-red) and low 'b' (blue-yellow)
    cv::Scalar lower_red(20, 150, 130); // L, a, b
    cv::Scalar upper_red(255, 255, 180);

    // Step 3: Threshold to get red mask
    cv::inRange(lab, lower_red, upper_red, mask);

    // Step 4: Morphological cleanup
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    // Step 5: Convert mask to BGR for visualization
    cv::cvtColor(mask, bgr, cv::COLOR_GRAY2BGR);

    // Step 6: Find contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    int leftCount = 0, centerCount = 0, rightCount = 0;
    cv::Mat outputFrame = frame.clone();
    const double minArea = 300;

    for (size_t i = 0; i < contours.size(); i++) {
        cv::Rect boundingBox = cv::boundingRect(contours[i]);
        double area = boundingBox.area();

        if (area < minArea) {
            continue;
        }

        int centerX = boundingBox.x + boundingBox.width / 2;
        const double aspectRatioToleranceCenter = 0.3;
        const double aspectRatioToleranceSide = 0.6;
        bool isValidRegion = false;

        if (centerX >= screenWidth / 3 && centerX < 2 * screenWidth / 3) {
            double aspectRatio = static_cast<double>(boundingBox.width) / boundingBox.height;
            if (std::abs(aspectRatio - 1.0) <= aspectRatioToleranceCenter) {
                isValidRegion = true;
            }
        }
        else {
            std::vector<cv::Point> approx;
            double epsilon = 0.04 * cv::arcLength(contours[i], true);
            cv::approxPolyDP(contours[i], approx, epsilon, true);

            if (approx.size() == 4 && cv::isContourConvex(approx)) {
                double aspectRatio = static_cast<double>(boundingBox.width) / boundingBox.height;
                if (std::abs(aspectRatio - 1.0) <= aspectRatioToleranceSide) {
                    isValidRegion = true;
                }
            }
        }

        if (isValidRegion) {
            cv::rectangle(outputFrame, boundingBox, cv::Scalar(0, 255, 0), 2);
            cv::rectangle(lab, boundingBox, cv::Scalar(0, 255, 0), 2);
            cv::rectangle(bgr, boundingBox, cv::Scalar(0, 255, 0), 2);

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
    if (frameCounter >= 15) { // Start showing stats after 10 frames
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

        // Check if best result changed and trigger TTS
        if (kEnableTts && bestResult != previousBestResult && bestResult != "Low confidence") {
            std::string guidanceMessage;

            // Create speech-friendly text
            if (bestResult.find("No valid red regions") != std::string::npos) {
                guidanceMessage = "No QR code detected. Please move around or adjust your camera.";
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

                    if (part.find("Left") != std::string::npos) {
                        messagePart = "on the left";
                    }
                    else if (part.find("Center") != std::string::npos) {
                        messagePart = "in the center";
                    }
                    else if (part.find("Right") != std::string::npos) {
                        messagePart = "on the right";
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
                }

                guidanceMessage = "Potential QR codes detected: " + combined +
                    ". Please move closer and adjust your camera to scan.";
            }

            // Use Text2Speech library
            narrate.speak(guidanceMessage);
            std::cout << guidanceMessage << std::endl;
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
    cv::putText(outputFrame, "Center", { leftSideWidth + screenWidth / 6, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(outputFrame, "Right", { rightSideWidth + leftSideWidth / 2, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);

    cv::line(lab, { leftSideWidth, 0 }, { leftSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::line(lab, { rightSideWidth, 0 }, { rightSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::putText(lab, "Left", { leftSideWidth / 2 - 20, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(lab, "Center", { leftSideWidth + screenWidth / 6, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(lab, "Right", { rightSideWidth + leftSideWidth / 2, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);

    cv::line(bgr, { leftSideWidth, 0 }, { leftSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::line(bgr, { rightSideWidth, 0 }, { rightSideWidth, frame.rows }, { 0, 255, 0 }, 2);
    cv::putText(bgr, "Left", { leftSideWidth / 2 - 20, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(bgr, "Center", { leftSideWidth + screenWidth / 6, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);
    cv::putText(bgr, "Right", { rightSideWidth + leftSideWidth / 2, frame.rows - 20 }, cv::FONT_HERSHEY_SIMPLEX, 1, { 0, 255, 0 }, 2);

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