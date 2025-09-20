#include <iostream>
#include <opencv2/core/core.hpp> // Core functions
#include <opencv2/highgui/highgui.hpp> // GUI functions
#include <chrono>

// Partitioning
#include "Supp.h"

// Created Modules
#include "QRDetector.h"
#include "Navigator.h"
#include "TripManager.h"
#include "Text2Speech.h"
#include "UIForVI.h"
#include "GridRouteReader.h"
#include "MapVisualizer.h"

using namespace std;
using namespace cv;
using namespace std::chrono;

// Define wall detection state
enum WallState { NONE, LEFT_WALL, RIGHT_WALL };
static WallState lastWallState = NONE;

// Simple left/right wall risk estimator using edge density
static pair<double, double> estimateSideEdgeDensity(const Mat& frame) {
    if (frame.empty()) return { 0.0, 0.0 };
    Mat gray, blurred, edges;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    GaussianBlur(gray, blurred, Size(5, 5), 1.2);
    Canny(blurred, edges, 50, 150);

    int w = edges.cols;
    int h = edges.rows;
    if (w == 0 || h == 0) return { 0.0, 0.0 };

    // Use outer 30% bands on each side instead of full halves
    int sideWidth = max(1, (int)std::round(w * 0.30));
    Rect leftRoi(0, 0, sideWidth, h);
    Rect rightRoi(w - sideWidth, 0, sideWidth, h);
    Mat left = edges(leftRoi);
    Mat right = edges(rightRoi);

    double leftCount = countNonZero(left);
    double rightCount = countNonZero(right);
    double leftDensity = leftCount / (left.total());
    double rightDensity = rightCount / (right.total());
    return { leftDensity, rightDensity };
}

// Partition Config
int const imgPerCol = 1, imgPerRow = 4;
Mat largeWin, win[imgPerCol * imgPerRow],
legend[imgPerCol * imgPerRow];

void onMouseClick(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        // Cast the userdata pointer back to an int pointer to get the cellSize
        int cellSize = *(static_cast<int*>(userdata));

        if (cellSize > 0) {
            int gridX = x / cellSize;
            int gridY = y / cellSize;
            // Print in the correct format for NodeMapping.txt
            std::cout << "Coordinate " << gridY << " " << gridX << std::endl;
        }
    }
}

int main() {
    VideoCapture cap(0);    // 0 (Default camera), >= 1 (other input)
    if (!cap.isOpened()) {
        cerr << "Camera not accessible.\n";
        return -1;
    }

    // Create module object
    QRDetector qrDetector;
    TripManager tripManager;
    Navigator navigator;
    Text2Speech narrate;
    UIForVI uiManager;
    GridRouteReader reader; // This object reads the grid and creates the graph

    string destNode;
    string destination;
    bool isNavigating = false;
    auto lastFeedbackTime = high_resolution_clock::now();
    auto lastWallWarnTime = high_resolution_clock::now();

    try {
        reader.gridLoader("NavigationFile\\RouteGrid.txt");
    }
    catch (Exception e) {
        narrate.speak("Error reading RouteGrid.txt");
    }

    try {
        navigator.loadNodeMap("NavigationFile\\NodeMapping.txt", reader.getGridWidth());;
    }
    catch (Exception e) {
        narrate.speak("Error reading NodeMapping.txt");
    }

    MapVisualizer mapViz(reader.getGrid(), navigator.getNodeCoordinates());
    vector<int> currentIntPath;
    string startNode = "";
    int frameCount = 0;
    string lastLocationID = "";

    // --- Add these lines for the mapping tool ---
    const std::string mapWindowName = "Navigation Map";
    cv::namedWindow(mapWindowName); // Create a named window

    // This is the key line to connect the mouse click to your function
    // We pass the cellSize from the visualizer to the callback
    cv::setMouseCallback(mapWindowName, onMouseClick, &mapViz.cellSize);

    narrate.speak("System initialized.");

    while (true) {
        frameCount++;

        Mat frame;
        cap >> frame;
        if (frame.empty()) continue;

        // Create partition
        createWindowPartition(frame, largeWin, win, legend, imgPerCol, imgPerRow);
        putText(legend[0], "Original RGB", Point(5, 11), 1, 1, Scalar(250, 250, 250), 1);
        putText(legend[1], "HSV Filter", Point(5, 11), 1, 1, Scalar(250, 250, 250), 1);
        putText(legend[2], "Mask BGR", Point(5, 11), 1, 1, Scalar(250, 250, 250), 1);
        putText(legend[3], "Segmentation", Point(5, 11), 1, 1, Scalar(250, 250, 250), 1);
        frame.copyTo(win[0]);

        string locationID;
        cv::Rect redBox = qrDetector.detect(frame, locationID);

        // Wall detection (run regardless of QR)
        auto now = high_resolution_clock::now();
        duration<double> since = duration_cast<duration<double>>(now - lastWallWarnTime);

        // Check every 2 seconds to avoid overly frequent feedback
        if (since.count() > 2.0) {
            auto densities = estimateSideEdgeDensity(frame);
            double leftD = densities.first;
            double rightD = densities.second;

            // Thresholds (tunable)
            double minDensityForWall = 0.02;   // Minimum edge density to consider a wall
            double triggerImbalance = 0.02;   // Trigger threshold (stricter)
            double releaseImbalance = 0.01;   // Release threshold (looser, for hysteresis)

            // Debug info
            char buf[128];
            snprintf(buf, sizeof(buf), "Wall L: %.3f  R: %.3f  dR-dL: %.3f",
                leftD, rightD, rightD - leftD);
            putText(legend[3], buf, Point(5, 26), 1, 1, Scalar(50, 200, 50), 1);
            cout << buf << endl;

            // show real-time visualization
            Mat gray, blurred, edges;
            cvtColor(frame, gray, COLOR_BGR2GRAY);
            GaussianBlur(gray, blurred, Size(5, 5), 1.2);
            Canny(blurred, edges, 50, 150);

            int w = edges.cols;
            int h = edges.rows;
            int sideWidth = max(1, (int)std::round(w * 0.30));

            // Left/Right ROI
            Rect leftRoi(0, 0, sideWidth, h);
            Rect rightRoi(w - sideWidth, 0, sideWidth, h);
            Mat leftEdge = edges(leftRoi).clone();
            Mat rightEdge = edges(rightRoi).clone();

            // Make visualization window
            cv::resize(leftEdge, leftEdge, cv::Size(200, 200));
            cv::resize(rightEdge, rightEdge, cv::Size(200, 200));

            Mat wallVis;
            hconcat(leftEdge, rightEdge, wallVis);
            cv::cvtColor(wallVis, wallVis, cv::COLOR_GRAY2BGR);

            putText(wallVis, "LEFT ROI", Point(20, 20), 1, 1, Scalar(0, 255, 0), 1);
            putText(wallVis, "RIGHT ROI", Point(220, 20), 1, 1, Scalar(0, 255, 0), 1);

            imshow("Wall Detection View", wallVis);

            // Decision logic (with hysteresis)
            if (rightD > minDensityForWall && (rightD - leftD) > triggerImbalance) {
                if (lastWallState != RIGHT_WALL) {
                    narrate.speak("Shift left a bit");
                    cout << "WARNING: Right wall detected!" << endl;
                    putText(frame, ">> RIGHT WALL <<", Point(50, 50), 2, 2, Scalar(0, 0, 255), 3);
                    lastWallState = RIGHT_WALL;
                    lastWallWarnTime = now;
                }
            }
            else if (leftD > minDensityForWall && (leftD - rightD) > triggerImbalance) {
                if (lastWallState != LEFT_WALL) {
                    narrate.speak("Shift right a bit");
                    cout << "WARNING: Left wall detected!" << endl;
                    putText(frame, "<< LEFT WALL >>", Point(frame.cols - 350, 50), 2, 2, Scalar(0, 0, 255), 3);
                    lastWallState = LEFT_WALL;
                    lastWallWarnTime = now;
                }
            }
            else {
                // When wall condition disappears (with hysteresis margin), reset state to NONE
                if (lastWallState == LEFT_WALL && (leftD - rightD) < releaseImbalance) {
                    lastWallState = NONE;
                }
                else if (lastWallState == RIGHT_WALL && (rightD - leftD) < releaseImbalance) {
                    lastWallState = NONE;
                }
            }
        }

        if (!locationID.empty()) {
            // Check for short directional commands first
            if (locationID == "TL") {
                narrate.speak("Turn left");
                cout << "Directional command: Turn left" << endl;
            }
            else if (locationID == "TR") {
                narrate.speak("Turn right");
                cout << "Directional command: Turn right" << endl;
            }
            // Only act if the location is new and not a directional command
            else if (locationID != lastLocationID) {
                lastLocationID = locationID; // Update last seen location immediately
                narrate.speak("You're at " + locationID);

                if (isNavigating) {
                    // --- NAVIGATION IN PROGRESS ---

                    // 1. Check if we are ON the planned path
                    if (tripManager.isLocationOnPath(locationID)) {
                        tripManager.updateLocation(locationID);

                        // Check for arrival right after updating
                        if (tripManager.isPathEmpty()) {
                            narrate.speak("Arrived at " + destination);
                            isNavigating = false;
                            destination = "";
                            currentIntPath.clear();
                            lastLocationID = ""; // <<< ADD THIS LINE TO RESET THE STATE
                        }
                    }
                    // 2. If we are OFF the planned path, trigger a reroute
                    else {
                        narrate.speak("Rerouting from " + locationID);
                        cout << "Rerouting from " << locationID << " to " << destination << endl;

                        currentIntPath = navigator.findPath(locationID, destination, reader.getGraph(), reader.getGridWidth());

                        if (!currentIntPath.empty()) {
                            auto stringPath = navigator.convertPathToStrings(currentIntPath, reader.getGridWidth());
                            tripManager.setPath(stringPath);
                            tripManager.updateLocation(locationID); // Update with the new path

                            string nextNode = tripManager.getNextNode();
                            if (!nextNode.empty()) {
                                narrate.speak("Reroute successful. Next stop is " + nextNode);
                            }
                            else {
                                narrate.speak("Reroute successful. Proceed to destination " + destination);
                            }
                        }
                        else {
                            narrate.speak("Sorry, I could not find a new path from here.");
                            isNavigating = false;
                            destination = "";
                            currentIntPath.clear();
                        }
                    }
                }
                else {
                    // --- START A NEW TRIP ---
                    startNode = locationID;
                    cout << "\nStart node   : " << locationID << endl;
                    destination = uiManager.selectDestination(locationID);

                    if (destination != "" && destination != "Cancel") {
                        currentIntPath = navigator.findPath(locationID, destination, reader.getGraph(), reader.getGridWidth());

                        if (!currentIntPath.empty()) {
                            auto stringPath = navigator.convertPathToStrings(currentIntPath, reader.getGridWidth());
                            tripManager.setPath(stringPath);
                            tripManager.updateLocation(locationID);

                            isNavigating = true;
                            cout << "End node     : " << destination << endl;

                            string nextNode = tripManager.getNextNode();
                            if (!nextNode.empty()) {
                                narrate.speak("Navigating to " + destination + ". Next stop is " + nextNode);
                            }
                            else {
                                narrate.speak("Navigating to " + destination);
                            }
                            startNode = "";
                        }
                        else {
                            narrate.speak("Sorry, I could not find a path to " + destination);
                            startNode = "";
                        }
                    }
                }
            } // ADD THE "ELSE IF" right after this closing brace
            else if (isNavigating && redBox.area() > 0) {
                auto now = high_resolution_clock::now();
                duration<double> time_span = duration_cast<duration<double>>(now - lastFeedbackTime);

                // Provide feedback only every 2 seconds to avoid overwhelming the user
                if (time_span.count() > 2.0) {
                    // --- Directional Guidance (Left/Right) ---
                    // Get the center of the camera frame
                    int frame_center_x = frame.cols / 2;
                    // Get the center of the detected red box
                    int red_box_center_x = redBox.x + redBox.width / 2;

                    // Define a tolerance zone around the center
                    int tolerance = frame.cols / 10; // e.g., 10% of the frame width

                    if (red_box_center_x < frame_center_x - tolerance) {
                        narrate.speak("Turn slightly left");
                        lastFeedbackTime = now;
                    }
                    else if (red_box_center_x > frame_center_x + tolerance) {
                        narrate.speak("Turn slightly right");
                        lastFeedbackTime = now;
                    }

                    // --- Distance Guidance (Forward/Backward) ---
                    double frame_area = frame.cols * frame.rows;
                    double red_box_area = redBox.area();
                    double area_ratio = red_box_area / frame_area;

                    // These thresholds may need tuning based on your camera and environment
                    if (area_ratio > 0.15) { // If the red area takes up > 15% of the view
                        narrate.speak("You are too close to the wall");
                        lastFeedbackTime = now;
                    }
                    else if (area_ratio < 0.01) { // If the red area takes up < 1% of the view
                        narrate.speak("You are too far from the wall");
                        lastFeedbackTime = now;
                    }
                }
            }
        }

        resize(largeWin, largeWin, cv::Size(1400, 320));
        imshow("Image Processing", largeWin);

        bool blinkState = (frameCount / 20) % 2 == 1; // Blink every 20 frames
        Mat mapView = mapViz.drawMap(tripManager.getCurrentLocation(), destination, currentIntPath, reader.getGridWidth(), startNode, blinkState);
        if (!mapView.empty()) {
            imshow(mapWindowName, mapView);
            imshow("Navigation Map", mapView);
        }

        int key = waitKey(1);

        // If ANY key is pressed (waitKey returns something other than -1),
        // stop the current narration.
        if (key != -1) {
            narrate.stop();
        }
        if (key == 'x') {
            tripManager.setPath(vector<string>{});
            isNavigating = false;
            startNode = "";
            currentIntPath.clear();
            narrate.speak("Navigation cancelled");
        }
        if (key == 'q') break;
    }
}

