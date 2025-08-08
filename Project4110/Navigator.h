#pragma once
#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

class Navigator {
public:
    vector<string> findPath(const string& start, const string& end);
    void loadMap(const string& filename);
private:
    unordered_map<string, vector<pair<string, int>>> graph;
};
