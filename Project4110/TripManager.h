#pragma once
#include <vector>
#include <string>

class TripManager {
public:
    void updateLocation(const std::string& location);
    void setPath(const std::vector<std::string>& path);
    std::string getNextNode();
    bool hasPath() const;

private:
    std::string currentLocation;
    std::vector<std::string> path;
    size_t pathIndex = 0;
};
