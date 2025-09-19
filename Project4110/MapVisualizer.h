#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>
#include <map>

class MapVisualizer {
public:
    MapVisualizer(const std::vector<std::string>& grid, const std::map<std::string, cv::Point>& nodeCoords);
    int cellSize;
    cv::Mat drawMap(
        const std::string& currentPosNode,
        const std::string& destNode,
        const std::vector<int>& route,
        int gridWidth,
        const std::string& startNode,
        bool blinkState
    );

private:
    cv::Mat mapBase;
    std::map<std::string, cv::Point> nodeCoordinates;
    int gridRows;
    int gridCols;
};