#include "TripManager.h"

void TripManager::updateLocation(const std::string& location) {
    currentLocation = location;
    if (pathIndex < path.size() && path[pathIndex] == location) {
        pathIndex++;
    }
}

void TripManager::setPath(const std::vector<std::string>& newPath) {
    path = newPath;
    pathIndex = 0;
}

std::string TripManager::getNextNode() {
    if (pathIndex < path.size()) return path[pathIndex];
    return "";
}

bool TripManager::hasPath() const {
    return !path.empty();
}
