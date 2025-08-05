#pragma once
#include <vector>
#include <string>
#include <unordered_map>

class Navigator {
public:
    std::vector<std::string> findPath(const std::string& start, const std::string& end);
    void loadMap(const std::string& filename);
private:
    std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> graph;
};
