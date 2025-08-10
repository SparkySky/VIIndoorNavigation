#include "Navigator.h"
#include "GridRouteReader.h"
#include <queue>
#include <cmath>
#include <algorithm>
#include <fstream>
//#include "nlohmann/json.hpp"
#include <iostream>

using namespace std;

//using json = nlohmann::json;

// read grid
vector<string> GridRouteReader::gridLoader(const string& filename) {
    grid.clear();
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return {};
    }

    string line;
    while (getline(file, line)) {
        if (!line.empty())
        grid.push_back(line);
    }

    file.close();
    return grid;
}




//A*
struct Node {
    int x, y;
    float g, h;
    float f() const { return g + h; }
};

struct NodeCompare {
    bool operator()(const Node& a, const Node& b) const {
        return a.f() > b.f();
    }
};

const vector<pair<int, int>> directions = {
    {-1, 0}, {-1, 1}, {0, 1}, {1, 1},
    {1, 0}, {1, -1}, {0, -1}, {-1, -1}
};

inline bool isValid(int x, int y, int rows, int cols, const vector<vector<int>>& grid) {
    return x >= 0 && y >= 0 && x < rows && y < cols && grid[x][y] == 0;
}

inline float heuristic(int x1, int y1, int x2, int y2) {
    return sqrtf((x1 - x2) * (x1 - x2) +
                 (y1 - y2) * (y1 - y2));
}

vector<pair<int, int>> aStarSearch(
    const vector<vector<int>>& grid,
    pair<int, int> start,
    pair<int, int> goal
) {
    int rows = grid.size();
    int cols = grid[0].size(); //crashed
    if (grid.empty() || grid[0].empty()) return {};


    vector<vector<float>> gCost(rows, vector<float>(cols, numeric_limits<float>::infinity()));
    vector<vector<bool>> closed(rows, vector<bool>(cols, false));
    vector<vector<pair<int, int>>> parent(rows, vector<pair<int, int>>(cols, {-1, -1}));

    priority_queue<Node, vector<Node>, NodeCompare> openList;

    Node startNode = {start.first, start.second, 0.0f, heuristic(start.first, start.second, goal.first, goal.second), nullptr};
    openList.push(startNode);
    gCost[start.first][start.second] = 0.0f;

    while (!openList.empty()) {
        Node current = openList.top();
        openList.pop();

        if (current.x == goal.first && current.y == goal.second) {
            vector<pair<int, int>> path;
            int cx = current.x;
            int cy = current.y;
            while (cx != -1 && cy != -1) {
                path.push_back({cx, cy});
                auto p = parent[cx][cy];
                cx = p.first;
                cy = p.second;
            }
            reverse(path.begin(), path.end());
            return path;
        }

        if (closed[current.x][current.y]) continue;
        closed[current.x][current.y] = true;

        for (auto& dir : directions) {
            int nx = current.x + dir.first;
            int ny = current.y + dir.second;

            if (!isValid(nx, ny, grid) || closed[nx][ny]) continue;

            const float STRAIGHT_COST = 1.0f;
            const float DIAGONAL_COST = 1.414f;

            float tentativeG = gCost[current.x][current.y] + moveCost;

            if (tentativeG < gCost[nx][ny]) {
                gCost[nx][ny] = tentativeG;
                parent[nx][ny] = {current.x, current.y};
                float h = heuristic(nx, ny, goal.first, goal.second);
                openList.push({nx, ny, tentativeG, h, nullptr});
            }
        }
    }

    return {};
}
