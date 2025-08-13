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
    GridRouteReader reader;

    string destNode;
    string destination;
    bool isNavigating = 0;

    reader.gridLoader("NavigationFile\\RouteGrid.txt");
    narrate.speak("System initialized.");

    while (true) {
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
                if (isNavigating) { // If previously is navigating, and now no more node = destination reached
                    narrate.speak("Arrived at " + destination);
                    isNavigating = 0;
                }
                
                cout << "\nStart node   : " << locationID << endl; // For Debug Nodes 
                destination = uiManager.selectDestination(locationID);
                if (destination != "") {
                    // Set up navigation route
                    auto path = navigator.findPath(locationID, destination); 
                    tripManager.setPath(path);
                    isNavigating = 1;
                    cout << "End node     : " << destination << endl; // For Debug Nodes
                    narrate.speak("Walk towards " + destination);
                }
            }

        }

        resize(largeWin, largeWin, cv::Size(1400, 320));
        imshow("Image Processing", largeWin);   // Show Process 
        if (waitKey(1) == 'x') {
            locationID = "";
            destNode = "";
            tripManager.setPath(vector<string> {});
            narrate.speak("Navigation cancelled");
        }
        if (waitKey(1) == 'q') break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}