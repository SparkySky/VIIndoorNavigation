#define NOMINMAX

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <algorithm>

#include "ColourProcessing.h"

using namespace cv;
using namespace std;

boolean justLaunch = 1;
ColourProcessing::ColourProcessing() {}
extern Mat win[];

cv::Mat ColourProcessing::extractRedRegion(const cv::Mat& frame, ShapeAnalysis& analyser) {
    // Configure batch size here - change this value as needed
    const int kBatchSize = 5;

    // Configure confidence threshold (percentage) - only show results above this confidence
    const double kConfidenceThreshold = 20;

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

    // Convert to CIELAB color space
    cv::cvtColor(frame, lab, cv::COLOR_BGR2Lab);

    // Manual Adjust
    cv::Scalar lower_red(10, 140, 125);
    cv::Scalar upper_red(255, 255, 185);


    // Threshold to get red mask
    cv::inRange(lab, lower_red, upper_red, mask);

    // Morphological cleanup
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);

    int leftCount = 0, centerCount = 0, rightCount = 0;
    cv::cvtColor(mask, bgr, cv::COLOR_GRAY2BGR); // Prepare BGR mask for drawing

    // Call the new module to do the actual shape detection and drawing
    analyser.shapeRecognition(mask, outputFrame, lab, bgr, leftCount, centerCount, rightCount);

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
    if (frameCounter >= 5) { // Start showing stats after 5 frames
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

        const int kStabilityThreshold = 10; // Require 5 stable frames before speaking (about 0.5s)

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
