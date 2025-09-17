#pragma once
#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <opencv2/core/types.hpp> // Required for cv::Point
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include "Graph.h"


class Navigator {
public:
    Navigator();
    bool loadNodeMap(const std::string& filename, int gridWidth);
    std::vector<int> findPath(const std::string& startNodeName, const std::string& endNodeName, const Graph& graph, int gridWidth); // Pass gridWidth
    std::vector<std::string> convertPathToStrings(const std::vector<int>& path, int gridWidth);
    const std::map<std::string, cv::Point>& getNodeCoordinates() const;
private:
    std::map<std::string, cv::Point> nodeCoordinates;
    // This map will be the reverse of nodeCoordinates to easily look up names from coordinates
    std::map<int, std::string> indexToNodeName;
};

#endif