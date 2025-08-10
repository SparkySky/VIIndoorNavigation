#pragma once

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>

using namespace std;

class GridRouteReader {
public:

    vector<string> gridLoader(const string& filename);

    vector<pair<int, int>> findPath(pair<int, int> start, pair<int, int> goal) const;

    unordered_map<string, pair<int,int>> landmarks;

private:
    vector<string> grid;
};
