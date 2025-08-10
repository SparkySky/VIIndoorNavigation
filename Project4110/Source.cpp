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
#include "UIforVI.h"
#include "GridRouteReader"

using namespace std;
using namespace cv;

// Partition Config
int const imgPerCol = 1, imgPerRow = 3;
Mat largeWin, win[imgPerCol * imgPerRow],
    legend[imgPerCol * imgPerRow];

int main() {
    GridRouteReader reader;
    reader gridLoader("NavigationFile\RouteGrid.txt");
   vector<vector<int>> grid(gridData.size(), vector<int>(gridData[0].size(), 0));
    for (int i = 0; i < gridData.size(); i++) {
        for (int j = 0; j < gridData[i].size(); j++) {
            grid[i][j] = (gridData[i][j] == '1');
        }
    }

    pair<int,int> start = {0, 0};
    pair<int,int> goal = {99, 99};

    auto path = aStar(grid, start, goal);

    for (auto [x, y] : path) {
        cout << "(" << x << "," << y << ") ";
    }
    
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Camera not accessible.\n";
        return -1;
    }

    // Create module object
    QRDetector qrDetector;
    TripManager tripManager;
    Navigator navigator;
    Text2Speech tts;
    UIforVI uiManager;

    tts.speak("System initialized.");

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) continue;

        createWindowPartition(frame, largeWin, win, legend, imgPerCol, imgPerRow);
        frame.copyTo(win[0]);

        

        std::string locationID = qrDetector.detect(frame);
        //cv::QRCodeDetector qrDecoder;
        //std::string locationID = qrDecoder.detectAndDecode(frame);
        std::cout << "QR data: " << locationID << std::endl;

        if (!locationID.empty()) {
            cout << "Run 2\n";
            tripManager.updateLocation(locationID);
            tts.speak("You are at " + locationID);

            std::string destNode = tripManager.getNextNode();

            if (!tripManager.hasPath() || destNode=="") {
                std::string destination = uiManager.selectDestination(locationID);
                auto path = navigator.findPath(locationID, destination);
                tripManager.setPath(path);
            }


            tts.speak("Walk towards " + destNode);
        }

        imshow("Before and after", largeWin);
        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}


