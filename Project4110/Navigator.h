
#pragma once
#include <vector>
#include <utility>
#include <string>

using namespace std;

struct PathResult {
    vector<pair<int, int>> path;
    vector<string> directions;
};

class Navigator {
public:
    vector<pair<int, int>> findPath(
        const vector<vector<int>>& grid,
        pair<int, int> start,
        pair<int, int> goal
    );

};
