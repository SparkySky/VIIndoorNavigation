#pragma once

#include <string>
#include <vector>

using namespace std;

class GridRouteReader {
public:

    vector<string> gridLoader(const string& filename);

    vector<pair<int, int>> findPath(pair<int, int> start, pair<int, int> goal) const;

private:
    vector<string> grid;
};
