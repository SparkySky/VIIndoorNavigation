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

        

        string locationID = qrDetector.detect(frame);

        /* Default QR Scanner */
        //QRCodeDetector qrDecoder;
        //string locationID = qrDecoder.detectAndDecode(frame);

        // cout << "QR data: " << locationID << endl; // For Debug QR

        if (!locationID.empty()) {
            tripManager.updateLocation(locationID);
            narrate.speak("You're at " + locationID);

            destNode = tripManager.getNextNode();

            // If path not exist, the next QR scanned will be used to initialize
            // the navigation starting point.
            // BACKUP: if (!tripManager.hasPath() || destNode == "") {
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
        imshow("Before and after", largeWin);

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