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


vector<string> GridRouteReader::gridLoader(const string& filename) {
    grid.clear();
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return {};
    }

    string line;
    int lineno = 0;  //   show grid
    while (getline(file, line)) {
        cout << lineno << ": '" << line << "'" << endl; // show grid
        lineno++; // show grid
        if (!line.empty())
            grid.push_back(line);
    }
    file.close();

	// landmarks (row, col), starts with (0, 0) ends with (99, 99)
    landmarks["Library"] = { 18, 4  };
    landmarks["Fountain"] = { 55, 97 };


    return grid;
}


vector<vector<int>> GridRouteReader::toIntGrid(const vector<string>& grid) const {
    vector<vector<int>> intGrid;
    for (const auto& row : grid) {
        vector<int> intRow;
        for (char c : row) {
            intRow.push_back(c == '1' ? 1 : 0);
        }
        intGrid.push_back(intRow);
    }
    return intGrid;
}

//astar
vector<pair<int, int>> Navigator::findPath( const vector<vector<int>>& grid, pair<int, int> start, pair<int, int> goal) {
    if (grid.empty() || grid[0].empty()) return {};

    int rows = grid.size();
    int cols = grid[0].size();

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

    

    auto isValid = [&](int x, int y) {
        return x >= 0 && y >= 0 && x < rows && y < cols && grid[x][y] == 0;
        };

    auto heuristic = [&](int x1, int y1, int x2, int y2) {
        return sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
        };

    vector<vector<float>> gCost(rows, vector<float>(cols, numeric_limits<float>::infinity()));
    vector<vector<bool>> closed(rows, vector<bool>(cols, false));
    vector<vector<pair<int, int>>> parent(rows, vector<pair<int, int>>(cols, { -1, -1 }));

    priority_queue<Node, vector<Node>, NodeCompare> openList;

    gCost[start.first][start.second] = 0.0f;
    openList.push({ start.first, start.second, 0.0f, heuristic(start.first, start.second, goal.first, goal.second) });

    while (!openList.empty()) {
        Node current = openList.top();
        openList.pop();

        if (current.x == goal.first && current.y == goal.second) {
            vector<pair<int, int>> path;
            int cx = current.x;
            int cy = current.y;
            while (cx != -1 && cy != -1) {
                path.push_back({ cx, cy });
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

            if (!isValid(nx, ny) || closed[nx][ny]) continue;

            const float STRAIGHT_COST = 1.0f;
            const float DIAGONAL_COST = 1.414f;
            float moveCost = (dir.first != 0 && dir.second != 0) ? DIAGONAL_COST : STRAIGHT_COST;

            float tentativeG = gCost[current.x][current.y] + moveCost;

            if (tentativeG < gCost[nx][ny]) {
                gCost[nx][ny] = tentativeG;
                parent[nx][ny] = { current.x, current.y };
                float h = heuristic(nx, ny, goal.first, goal.second);
                openList.push({ nx, ny, tentativeG, h });
            }

        }
    }

    return {};
}

