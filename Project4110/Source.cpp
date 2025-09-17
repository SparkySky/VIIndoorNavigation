#include <iostream>
#include <opencv2/core/core.hpp> // Core functions
#include <opencv2/highgui/highgui.hpp> // GUI functions

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

// Partition Config
int const imgPerCol = 1, imgPerRow = 4;
Mat largeWin, win[imgPerCol * imgPerRow],
    legend[imgPerCol * imgPerRow];

int main() {
    VideoCapture cap(1);    // 0 (Default camera), >= 1 (other input)
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

    try {
        reader.gridLoader("RouteGrid.txt");
    }
    catch (Exception e) {
        narrate.speak("Error reading RouteGrid.txt");
    }

    try {
        navigator.loadNodeMap("NodeMapping.txt", reader.getGridWidth());;
    }
    catch (Exception e) {
        narrate.speak("Error reading NodeMapping.txt");
    }

    MapVisualizer mapViz(reader.getGrid(), navigator.getNodeCoordinates());
    vector<int> currentIntPath;
    string startNode = "";
    int frameCount = 0;
    string lastLocationID = "";

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

        string locationID = qrDetector.detect(frame);

        if (!locationID.empty()) {
            // Only act if the location is new
            if (locationID != lastLocationID) {
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
                        narrate.speak("Off course. Rerouting from " + locationID);
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
                                narrate.speak("Reroute successful. Proceed to " + destination);
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
            }
        }

        resize(largeWin, largeWin, cv::Size(1400, 320));
        imshow("Image Processing", largeWin);

        bool blinkState = (frameCount / 20) % 2 == 1; // Blink every 20 frames
        Mat mapView = mapViz.drawMap(tripManager.getCurrentLocation(), destination, currentIntPath, reader.getGridWidth(), startNode, blinkState);
        if (!mapView.empty()) {
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