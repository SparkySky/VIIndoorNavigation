#include "QRDetector.h"
#include "Navigator.h"
#include "TripManager.h"
#include "Text2Speech.h"
#include "UIforVI.h"
#include "GridRouteReader"

#include <iostream>
#include	<opencv2/core/core.hpp> // The core functions of opencv
#include	<opencv2/highgui/highgui.hpp> // The GUI functions of opencv

using namespace std;
using namespace cv;

int main() {
    GridRouteReader reader;
    reader gridLoader("NavigationFile\RouteGrid.txt");
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Camera not accessible.\n";
        return -1;
    }

    QRDetector qrDetector;
    TripManager tripManager;
    Navigator navigator;
    Text2Speech tts;
    UIforVI uiManager;

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) continue;

        tts.speak("System initialized.");

        std::string locationID = qrDetector.detect(frame);
        //cv::QRCodeDetector qrDecoder;
        //std::string locationID = qrDecoder.detectAndDecode(frame);
        std::cout << "QR data: " << locationID << std::endl;

        if (!locationID.empty()) {
            cout << "Run 2\n";
            tripManager.updateLocation(locationID);
            tts.speak("You are at " + locationID);

            if (!tripManager.hasPath()) {
                std::string destination = uiManager.selectDestination();
                auto path = navigator.findPath(locationID, destination);
                tripManager.setPath(path);
            }

            std::string nextNode = tripManager.getNextNode();
            tts.speak("Walk towards " + nextNode);
        }

        if (cv::waitKey(1) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}

