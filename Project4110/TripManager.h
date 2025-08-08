#pragma once
#include <vector>
#include <string>
#include "Text2Speech.h"

using namespace std;

class TripManager {
public:
    void updateLocation(const string& location);
    void setPath(const vector<string>& path);
    bool hasPath() const;
    string getNextNode();
private:
    string currentLocation;
    vector<string> path;
    size_t pathIndex = 0;
};
