#include "MapVisualizer.h"
#include <iostream>

MapVisualizer::MapVisualizer(const std::vector<std::string>& grid, const std::map<std::string, cv::Point>& nodeCoords, int cellSize) {
    this->nodeCoordinates = nodeCoords;
    this->cellSize = cellSize;
    this->gridRows = grid.size();
    if (gridRows > 0) {
        // Calculate columns based on the first row, assuming a rectangular grid
        int count = 0;
        for (char c : grid[0]) {
            if (c == '0' || c == '1') {
                count++;
            }
        }
        this->gridCols = count;
    }
    else {
        this->gridCols = 0;
    }

    // Create the base map image once
    this->mapBase = cv::Mat::zeros(gridRows * cellSize, gridCols * cellSize, CV_8UC3);

    for (int r = 0; r < gridRows; ++r) {
        for (int c = 0; c < gridCols; ++c) {
            // Find the character for the grid cell, skipping spaces
            int char_idx = c * 2; // "1 0 1 " format
            if (char_idx < grid[r].length()) {
                cv::Point p1(c * cellSize, r * cellSize);
                cv::Point p2((c + 1) * cellSize, (r + 1) * cellSize);
                // Draw walls as gray, paths as off-white
                cv::Scalar color = (grid[r][char_idx] == '1') ? cv::Scalar(100, 100, 100) : cv::Scalar(240, 240, 240);
                cv::rectangle(mapBase, p1, p2, color, -1);
            }
        }
    }

    // Draw all QR points as green circles on the base map
    for (const auto& pair : nodeCoordinates) {
        cv::Point center(pair.second.x * cellSize + cellSize / 2, pair.second.y * cellSize + cellSize / 2);
        cv::circle(mapBase, center, cellSize / 4, cv::Scalar(0, 180, 0), -1); // Green
    }
}

cv::Mat MapVisualizer::drawMap(const std::string& currentPosNode, const std::string& destNode, const std::vector<int>& route, int gridWidth, const std::string& startNode, bool blinkState) {
    cv::Mat frame = mapBase.clone();

    // Draw the route as a blue line
    if (route.size() > 1) {
        for (size_t i = 0; i < route.size() - 1; ++i) {
            int idx1 = route[i];
            int idx2 = route[i + 1];
            int r1 = idx1 / gridWidth, c1 = idx1 % gridWidth;
            int r2 = idx2 / gridWidth, c2 = idx2 % gridWidth;
            cv::Point p1(c1 * cellSize + cellSize / 2, r1 * cellSize + cellSize / 2);
            cv::Point p2(c2 * cellSize + cellSize / 2, r2 * cellSize + cellSize / 2);
            cv::line(frame, p1, p2, cv::Scalar(255, 0, 0), 3);
        }
    }

    // Draw destination point (Red)
    if (nodeCoordinates.count(destNode)) {
        cv::Point destPoint = nodeCoordinates.at(destNode);
        cv::Point center(destPoint.x * cellSize + cellSize / 2, destPoint.y * cellSize + cellSize / 2);
        cv::circle(frame, center, cellSize / 3, cv::Scalar(0, 0, 255), -1);
    }

    // Draw current position (Yellow, or blinking Cyan if it's the start node)
    if (nodeCoordinates.count(currentPosNode)) {
        cv::Point currentPoint = nodeCoordinates.at(currentPosNode);
        cv::Point center(currentPoint.x * cellSize + cellSize / 2, currentPoint.y * cellSize + cellSize / 2);

        cv::Scalar color = cv::Scalar(0, 255, 255); // Default Yellow
        if (currentPosNode == startNode && blinkState) {
            color = cv::Scalar(255, 255, 0); // Cyan for blinking start node
        }
        cv::circle(frame, center, cellSize / 3, color, -1);
    }

    return frame;
}