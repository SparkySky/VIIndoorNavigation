#include "Navigator.h"
#include "GridRouteReader.h"
#include <queue>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "nlohmann/json.hpp"
#include <iostream>

using namespace std;

using json = nlohmann::json;

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
        grid.push_back(line);
    }

    file.close();
    return grid;
}




//A*
struct Node {
    int x, y;
    double g, h;
    Node* parent;

    double f() const { return g + h; }

    Node(int x, int y, double g, double h, Node* parent = nullptr)
        : x(x), y(y), g(g), h(h), parent(parent) {}
};

struct CompareNode {
    bool operator()(const Node* a, const Node* b) {
        return a->f() > b->f();
    }
};

inline double heuristic(int x1, int y1, int x2, int y2) {
    double dx = abs(x1 - x2);
    double dy = abs(y1 - y2);
    return (dx + dy) + (sqrt(2) - 2) * min(dx, dy);
}

inline bool isValid(int x, int y, const vector<vector<int>>& grid) {
    return x >= 0 && y >= 0 &&
           x < (int)grid.size() &&
           y < (int)grid[0].size() &&
           grid[x][y] == 0;
}

vector<pair<int, int>> aStar(
    const vector<vector<int>>& grid,
    pair<int, int> start,
    pair<int, int> goal
) {
    priority_queue<Node*, vector<Node*>, CompareNode> openSet;
    vector<vector<bool>> closedSet(grid.size(), vector<bool>(grid[0].size(), false));

    Node* startNode = new Node(start.first, start.second, 0, heuristic(start.first, start.second, goal.first, goal.second));
    openSet.push(startNode);

    vector<pair<int, int>> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}, 
        {1, 1}, {-1, -1}, {1, -1}, {-1, 1}
    };

    while (!openSet.empty()) {
        Node* current = openSet.top();
        openSet.pop();

        if (current->x == goal.first && current->y == goal.second) {
            vector<pair<int, int>> path;
            while (current) {
                path.push_back({current->x, current->y});
                current = current->parent;
            }
            reverse(path.begin(), path.end());
            return path;
        }

        if (closedSet[current->x][current->y]) continue;
        closedSet[current->x][current->y] = true;

        for (auto& dir : directions) {
            int nx = current->x + dir.first;
            int ny = current->y + dir.second;

            if (isValid(nx, ny, grid) && !closedSet[nx][ny]) {
                double stepCost = (dir.first != 0 && dir.second != 0) ? sqrt(2) : 1;
                double newG = current->g + stepCost;
                double newH = heuristic(nx, ny, goal.first, goal.second);
                openSet.push(new Node(nx, ny, newG, newH, current));
            }
        }
    }

    return {};
}

vector<vector<int>> loadGridFromFile(const string& filename) {
    ifstream file(filename);
    vector<vector<int>> grid;
    string line;

    while (getline(file, line)) {
        istringstream iss(line);
        vector<int> row;
        int value;
        while (iss >> value) {
            row.push_back(value);
        }
        if (!row.empty()) grid.push_back(row);
    }

    return grid;
}
