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

// --- Global variables for UI and Callbacks ---
UIForVI uiManager;
UIForVI terminalUI; // The non-blocking terminal UI
const string imageProcessingWindow = "Image Processing";
bool mouseSelectionConfirmed = false;

// Partition Config
int const imgPerCol = 1, imgPerRow = 4;
Mat largeWin, win[imgPerCol * imgPerRow],
    legend[imgPerCol * imgPerRow];

// Tool: For extracting grid coordinate
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

// Mouse callback that works with the non-blocking UI
void onMouseCallback(int event, int x, int y, int flags, void* userdata) {
    if (uiManager.is_active()) {
        if (event == EVENT_MOUSEWHEEL) {
            int delta = getMouseWheelDelta(flags);
            if (delta > 0) uiManager.scroll_up();
            else uiManager.scroll_down();
        }
        if (event == EVENT_LBUTTONDOWN) {
            // This is the crucial part: the callback simply sets a flag.
            // The main loop is responsible for acting on it.
            mouseSelectionConfirmed = true;
        }
    }
}

// Helper function to draw the current selection on the video feed
void drawSelectionOnScreen(Mat& image, const UIForVI& ui) {
    if (!ui.is_active()) return;
    Mat overlay;
    image.copyTo(overlay);
    rectangle(overlay, Rect(0, 0, image.cols, image.rows), Scalar(0, 0, 0), FILLED);
    addWeighted(overlay, 0.7, image, 0.3, 0, image);

    string selection = ui.get_current_selection();
    putText(image, "Selected: " + selection, Point(20, image.rows / 2), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0, 255, 255), 3);
}



int main() {
    //VideoCapture cap(2, CAP_DSHOW);    // 0 (Default camera), >= 1 (other input)
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Camera not accessible.\n";
        return -1;
    }

    // Create module object
    QRDetector qrDetector;
    TripManager tripManager;
    Navigator navigator;
    Text2Speech narrate;
    GridRouteReader reader; // This object reads the grid and creates the graph

    string destNode;
    string destination;
    bool isNavigating = false;
    auto lastFeedbackTime = high_resolution_clock::now();

    try {
        reader.gridLoader("NavigationFile\\RouteGrid.txt");
        navigator.loadNodeMap("NavigationFile\\NodeMapping.txt", reader.getGridWidth());;
    }
    catch (Exception e) {
        narrate.speak_high_priority("Error reading Navigation Files");
        return 1;
    }

    MapVisualizer mapViz(reader.getGrid(), navigator.getNodeCoordinates());
    vector<int> currentIntPath;
    string startNode = "";
    int frameCount = 0;
    string lastLocationID = "";
    string lastSpokenSelection = "";
    Mat selectionFrame;
    string finalDestination = "";

    // Tool: Mapping coordinate tool
    const std::string mapWindowName = "Navigation Map";

    // --- Window Setup ---
    namedWindow(imageProcessingWindow);
    setMouseCallback(imageProcessingWindow, onMouseCallback, nullptr);
    namedWindow("Navigation Map");
    createTrackbar("Min Area", imageProcessingWindow, &qrDetector.minAreaTrackbar, 20000);

    Mat blackPanel = Mat::zeros(selectionFrame.size(), selectionFrame.type());
    narrate.speak_low_priority("System initialized. Scan a QR Code to start.");
    while (true) {
        frameCount++;
        narrate.update_status();

        Mat frame;
        string locationID;
        cv::Rect redBox;

        if (!uiManager.is_active()) {
            cap >> frame;
            if (frame.empty()) continue;

            // Process the live frame
            createWindowPartition(frame, largeWin, win, legend, imgPerCol, imgPerRow);
            putText(legend[0], "Original RGB", Point(5, 11), 1, 1, Scalar(250, 250, 250), 1);
            putText(legend[1], "HSV Filter", Point(5, 11), 1, 1, Scalar(250, 250, 250), 1);
            putText(legend[2], "Mask BGR", Point(5, 11), 1, 1, Scalar(250, 250, 250), 1);
            putText(legend[3], "Segmentation", Point(5, 11), 1, 1, Scalar(250, 250, 250), 1);
            frame.copyTo(win[0]);
            redBox = qrDetector.detect(frame, locationID);
        }
        else {
            frame = blackPanel;
            selectionFrame.copyTo(win[0]);
        }

        // Check for a new, valid QR scan when NOT in selection mode
        if (!locationID.empty() && locationID != lastLocationID && !uiManager.is_active()) {
            lastLocationID = locationID;
            narrate.speak_high_priority("You are at " + locationID);

            if (isNavigating) {
                if (tripManager.isLocationOnPath(locationID)) {
                    tripManager.updateLocation(locationID);
                    if (tripManager.isPathEmpty()) {
                        narrate.speak_high_priority("Arrived at " + finalDestination);
                        isNavigating = false;
                        finalDestination = "";
                        startNode = "";
                        currentIntPath.clear();
                        lastLocationID = "";
                    }
                }
                else {
                    // --- CORRECTED REROUTING LOGIC ---
                    narrate.speak_high_priority("Rerouting from " + locationID);

                    // Use the reliable 'finalDestination' variable
                    currentIntPath = navigator.findPath(locationID, finalDestination, reader.getGraph(), reader.getGridWidth());

                    if (!currentIntPath.empty()) {
                        auto stringPath = navigator.convertPathToStrings(currentIntPath, reader.getGridWidth());
                        tripManager.setPath(stringPath);
                        tripManager.updateLocation(locationID);
                        narrate.speak_low_priority("Rerouted. Next stop is " + tripManager.getNextNode());
                    }
                    else {
                        narrate.speak_high_priority("Sorry, a new path could not be found from here.");
                        isNavigating = false;
                        finalDestination = "";
                        currentIntPath.clear();
                    }
                }
            }
            else {
                startNode = locationID;
                uiManager.activate(locationID);
                lastSpokenSelection = "";
                win[0].copyTo(selectionFrame);
            }
        }

        // Handle user input IF selection mode is active
        if (uiManager.is_active()) {
            // 1. Create a temporary copy of the frozen frame to draw on
            Mat tempDisplayFrame = selectionFrame.clone();

            // 2. Draw the selection menu onto the temporary copy
            drawSelectionOnScreen(tempDisplayFrame, uiManager);

            // --- ENHANCED FREEZE: Black out other panels ---
            Mat blackPanel = Mat::zeros(selectionFrame.size(), selectionFrame.type());
            blackPanel.copyTo(win[0]);         // Black out the first panel
            tempDisplayFrame.copyTo(win[1]);   // Put the frame WITH the menu in the second panel
            blackPanel.copyTo(win[2]);         // Black out the third panel
            blackPanel.copyTo(win[3]);         // Black out the fourth panel

            string currentSelection = uiManager.get_current_selection();
            if (currentSelection != lastSpokenSelection) {
                narrate.speak_low_priority("Option " + currentSelection);
                lastSpokenSelection = currentSelection;
            }

            char key = (char)waitKey(30);

            if (key == 'w' || key == 'W') uiManager.scroll_up();
            else if (key == 's' || key == 'S') uiManager.scroll_down();

            if (key == 'c' || key == 'C' || mouseSelectionConfirmed) {
                destination = uiManager.get_current_selection();
                uiManager.deactivate();
                mouseSelectionConfirmed = false;
            }
            imshow(imageProcessingWindow, largeWin);
        }
        else {
            waitKey(1);
        }

        // --- START NAVIGATION (handles a confirmed selection) ---
        if (!destination.empty() && !isNavigating) {
            if (destination != "Cancel") {
                // Store the chosen destination in our reliable variable
                finalDestination = destination;
                narrate.speak_high_priority("Navigating to " + finalDestination);
                currentIntPath = navigator.findPath(startNode, finalDestination, reader.getGraph(), reader.getGridWidth());
                if (!currentIntPath.empty()) {
                    auto stringPath = navigator.convertPathToStrings(currentIntPath, reader.getGridWidth());
                    tripManager.setPath(stringPath);
                    tripManager.updateLocation(startNode);
                    isNavigating = true;
                }
                else {
                    narrate.speak_high_priority("Sorry, path not found.");
                    finalDestination = ""; // Reset if path fails
                }
            }
            else {
                lastLocationID = "";
                narrate.speak_low_priority("Navigation cancelled.");
            }
            destination = ""; // Reset the temporary variable
        }

        // --- Step 4: Always draw the final display ---
        resize(largeWin, largeWin, cv::Size(1400, 320));
        imshow(imageProcessingWindow, largeWin);

        bool blinkState = (frameCount / 20) % 2 == 1;
        Mat mapView = mapViz.drawMap(tripManager.getCurrentLocation(), destination, currentIntPath, reader.getGridWidth(), startNode, blinkState);
        if (!mapView.empty()) {
            imshow("Navigation Map", mapView);
        }
    }
}