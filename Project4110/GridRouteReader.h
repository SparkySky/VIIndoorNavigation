#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;

class GridRouteReader {
public:
    vector<string> gridLoader(const string& filename);

    vector<pair<int, int>> findPath(pair<int, int> start, pair<int, int> goal) const;

    unordered_map<string, pair<int,int>> getLandmarks() const {
        return landmarks;
    }

    vector<vector<int>> toIntGrid(const vector<string>& stringGrid) const;

private:
    vector<string> grid;
    unordered_map<string, pair<int,int>> landmarks;
};
