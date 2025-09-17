#include "TripManager.h"
#include <algorithm>

using namespace std;

TripManager::TripManager() {}

// For path update
// Use cases:
// 1. Update path,
// 2. Pinpoint starting location for navigation.
void TripManager::updateLocation(const std::string& newLocation) {
    currentLocation = newLocation;

    // Find the new location anywhere in the current path
    auto it = std::find(path.begin(), path.end(), newLocation);

    // If the scanned location is found in our planned path...
    if (it != path.end()) {
        // Erase all previous steps up to and including the one we just scanned.
        path.erase(path.begin(), it + 1);
    }
}

// Set new path (After setting destination)
void TripManager::setPath(const vector<string>& newPath) {
    path = newPath;
}

// Get next node in a predefined path
std::string TripManager::getNextNode() const {
    if (!path.empty()) {
        return path.front();
    }
    return "";
}

const std::string& TripManager::getCurrentLocation() const {
    return currentLocation;
}

bool TripManager::isPathEmpty() const {
    return path.empty();
}

bool TripManager::isLocationOnPath(const std::string& location) const {
    // Use std::find to search the path vector for the location.
    // This correctly checks if the scanned location is a future checkpoint.
    return (std::find(path.begin(), path.end(), location) != path.end());
}