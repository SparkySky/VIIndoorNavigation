#pragma once
#include <vector>
#include <string>

class TripManager {
public:
    TripManager();
    void setPath(const std::vector<std::string>& newPath);
    void updateLocation(const std::string& newLocation);
    std::string getNextNode() const;
    const std::string& getCurrentLocation() const;
    bool isPathEmpty() const;
    bool isLocationOnPath(const std::string& location) const; // For reroute purpose

private:
    std::vector<std::string> path;
    std::string currentLocation;
};