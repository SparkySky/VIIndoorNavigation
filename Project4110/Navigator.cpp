#include <queue>
#include <fstream>
#include <sstream>   // For parsing strings
#include <iostream>

#include "Navigator.h"
#include "GridRouteReader.h"
#include "nlohmann/json.hpp"

using namespace std;


Navigator::Navigator() {}

bool Navigator::loadNodeMap(const string& filename, int gridWidth) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open node mapping file: " << filename << endl;
        return false;
    }

    string line;

    while (getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        stringstream ss(line);
        string nodeName;
        int row, col;

        if (ss >> nodeName >> row >> col) {
            nodeCoordinates[nodeName] = cv::Point(col, row);
            int index = row * gridWidth + col; // Now uses the correct width
            indexToNodeName[index] = nodeName;
            
        }
        cout << nodeName << " " << row << " " << col << endl;
    }
    cout << "Node map loaded successfully." << endl;
}

vector<string> Navigator::convertPathToStrings(const vector<int>& path, int gridWidth) {
    vector<string> stringPath;
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

vector<int> Navigator::findPath(const string& startNodeName, const string& endNodeName, const Graph& graph, int gridWidth) {
    if (nodeCoordinates.find(startNodeName) == nodeCoordinates.end() ||
        nodeCoordinates.find(endNodeName) == nodeCoordinates.end()) {
        cerr << "Error: Start or end node name not found in map." << endl;
        return {};
    }

    cv::Point startPoint = nodeCoordinates[startNodeName];
    cv::Point endPoint = nodeCoordinates[endNodeName];

    int startIndex = startPoint.y * gridWidth + startPoint.x;
    int endIndex = endPoint.y * gridWidth + endPoint.x;

    cout << "Finding path from " << startNodeName << " (" << startIndex << ") to "
        << endNodeName << " (" << endIndex << ")" << endl;

    return graph.astar(startIndex, endIndex, gridWidth);
}

const map<string, cv::Point>& Navigator::getNodeCoordinates() const {
    return nodeCoordinates;

}
