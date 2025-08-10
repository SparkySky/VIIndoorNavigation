
#pragma once
#include <vector>
#include <utility>

using namespace std;

class Navigator {
public:
    vector<pair<int, int>> findPath(
        const vector<vector<int>>& grid,
        pair<int, int> start,
        pair<int, int> goal
    );
};
