#include "TripManager.h"

using namespace std;

// For path update
// Use cases:
// 1. Update path,
// 2. Pinpoint starting location for navigation.
void TripManager::updateLocation(const string& location) {
    currentLocation = location;
    if (pathIndex < path.size() && path[pathIndex] == location) {
        pathIndex++;
    }
}

// Set new path (After setting destination)
void TripManager::setPath(const vector<string>& newPath) {
    path = newPath;
    pathIndex = 0;
}

// Get next node in a predefined path
string TripManager::getNextNode() {
    if (pathIndex < path.size()) 
        return path[pathIndex];
    return "";
}