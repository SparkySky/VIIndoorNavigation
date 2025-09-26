// Yee Jia Hao
#include <queue>
#include <fstream>
#include <sstream>   // For parsing strings
#include <iostream>

#include "Navigator.h"
#include "GridRouteReader.h"
#include "nlohmann/json.hpp"

using namespace std;


Navigator::Navigator() {}

// Loads node names and their grid coordinates from "NavigationFile/".
bool Navigator::loadNodeMap(const std::string& filename, int gridWidth) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open node mapping file: " << filename << std::endl;
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ss(line);
        std::string nodeName;
        int row, col;

        if (ss >> nodeName >> row >> col) {
            nodeCoordinates[nodeName] = cv::Point(col, row);
            int index = row * gridWidth + col;
            indexToNodeName[index] = nodeName;
            
        }
        cout << nodeName << " " << row << " " << col << endl;
    }
    std::cout << "Node map loaded successfully." << std::endl;
}

// Converts a path of grid indices into a list of named nodes (e.g. N001, N002)
std::vector<std::string> Navigator::convertPathToStrings(const std::vector<int>& path, int gridWidth) {
    std::vector<std::string> stringPath;
    for (int index : path) {
        // Find if this index corresponds to a named node
        if (indexToNodeName.count(index)) {
            // Only add named nodes to the final path instructions
            if (stringPath.empty() || stringPath.back() != indexToNodeName[index]) {
                stringPath.push_back(indexToNodeName[index]);
            }
        }
    }
    return stringPath;
}

// Finds shortest path between two named nodes using algorithm
std::vector<int> Navigator::findPath(const std::string& startNodeName, const std::string& endNodeName, const Graph& graph, int gridWidth) {
    if (nodeCoordinates.find(startNodeName) == nodeCoordinates.end() ||
        nodeCoordinates.find(endNodeName) == nodeCoordinates.end()) {
        std::cerr << "Error: Start or end node name not found in map." << std::endl;
        return {};
    }

    cv::Point startPoint = nodeCoordinates[startNodeName];
    cv::Point endPoint = nodeCoordinates[endNodeName];

    int startIndex = startPoint.y * gridWidth + startPoint.x;
    int endIndex = endPoint.y * gridWidth + endPoint.x;

    std::cout << "Finding path from " << startNodeName << " (" << startIndex << ") to "
        << endNodeName << " (" << endIndex << ")" << std::endl;

    return graph.dijkstra(startIndex, endIndex);
}

// Returns the map of node names to their grid coordinates.
const std::map<std::string, cv::Point>& Navigator::getNodeCoordinates() const {
    return nodeCoordinates;
}