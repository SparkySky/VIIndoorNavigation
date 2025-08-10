#include <iostream>
#include <opencv2/core/core.hpp> // Core functions
#include <opencv2/highgui/highgui.hpp> // GUI functions

// Partitioning
#include	"Supp.h"

// Created Modules
#include "QRDetector.h"
#include "Navigator.h"
#include "TripManager.h"
#include "Text2Speech.h"
#include "UIForVI.h"
#include "GridRouteReader.h"

using namespace std;
using namespace cv;

// Partition Config
int const imgPerCol = 1, imgPerRow = 2;
Mat largeWin, win[imgPerCol * imgPerRow],
    legend[imgPerCol * imgPerRow];

int main() {
    GridRouteReader reader;
    auto gridData = reader.gridLoader("NavigationFile/RouteGrid.txt");
    auto intGrid = reader.toIntGrid(gridData);
    auto landmarks = reader.getLandmarks();

    // Current position
    string scannedCurrentLandmark = "Library";  // REPLACE THIS WITH INPUT
    if (landmarks.find(scannedCurrentLandmark) == landmarks.end()) {
        cerr << "Error: scanned landmark not found!\n";
        return -1;
    }
    pair<int, int> currentPos = landmarks[scannedCurrentLandmark];
    cout << "Current position (Based on QR): " << scannedCurrentLandmark
         << " at (" << currentPos.first << ", " << currentPos.second << ")\n";

    // Destination position
    string scannedDestinationLandmark = "Fountain";  // REPLACE THIS WITH USER INPUT
    if (landmarks.find(scannedDestinationLandmark) == landmarks.end()) {
        cerr << "Error: destination landmark not found!\n";
        return -1;
    }
    pair<int, int> goal = landmarks[scannedDestinationLandmark];
    cout << "Destination position: " << scannedDestinationLandmark
         << " at (" << goal.first << ", " << goal.second << ")\n";

    Navigator astar;
    auto path = astar.findPath(intGrid, currentPos, goal);

    cout << "Path from current to destination:\n";
    for (auto [x, y] : path) {
        cout << "(" << x << "," << y << ") ";
    }
    cout << endl;

    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Camera not accessible.\n";
        return -1;
    }

    return 0;
}
    }

    // Create module object
    QRDetector qrDetector;
    TripManager tripManager;
    Navigator navigator;
    Text2Speech narrate;
    UIForVI uiManager;
    GridRouteReader reader;

    string destNode;

    reader.gridLoader("NavigationFile\\RouteGrid.txt");
    narrate.speak("System initialized.");

    while (true) {
        Mat frame;
        cap >> frame;
        if (frame.empty()) continue;

        // Create partition
        createWindowPartition(frame, largeWin, win, legend, imgPerCol, imgPerRow);
        frame.copyTo(win[0]);

        // Retrieving LocationID from QR Code using component developed by
        // Khor Jia En (Color Processing) and Yee Wen Xian (QR Detection)
        string locationID = qrDetector.detect(frame); 

        /* Default OpenCV QR Scanner */
        // QRCodeDetector qrDecoder;
        // string locationID = qrDecoder.detectAndDecode(frame);

        if (!locationID.empty()) {
            tripManager.updateLocation(locationID); // Constantly localizing user
            narrate.speak("You're at " + locationID);

            destNode = tripManager.getNextNode(); // Get next node in generated path

            // If path/node not exist before, the QR scanned will be used to 
            // initialize the navigation starting point.
            // BACKUP Code: if (!tripManager.hasPath() || destNode == "") {
            if (destNode == "") {
                cout << "Loc data: " << locationID << endl; // For Debug Nodes 
                string destination = uiManager.selectDestination(locationID);
                if (destination != "") {
                    // Set up navigation route
                    auto path = navigator.findPath(locationID, destination); 
                    tripManager.setPath(path); 

                    narrate.speak("Walk towards " + destination);
                }
            }

        }
        if (waitKey(1) == 'x') {
            locationID = "";
            destNode = "";
            tripManager.setPath(vector<string> {});
            narrate.speak("Navigation cancelled");
        }

        imshow("Before and after", largeWin);
        if (waitKey(1) == 'q') break;
    }

    cap.release();
    destroyAllWindows();
    return 0;

}
