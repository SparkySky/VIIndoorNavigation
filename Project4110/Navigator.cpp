#include "Navigator.h"
#include "GridRouteReader.h"
#include <queue>
#include <fstream>
#include "nlohmann/json.hpp"
#include <iostream>

using namespace std;

using json = nlohmann::json;

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




//djikstra
void Navigator::loadMap(const string& filename) {
    ifstream file(filename);
    json j;
    file >> j;
    for (auto& node : j.items()) {
        string id = node.key();
        for (auto& neighbor : node.value()) {
            graph[id].emplace_back(neighbor["id"], neighbor["cost"]);
        }
    }
}

vector<string> Navigator::findPath(const string& start, const string& end) {
    unordered_map<string, int> dist;
    unordered_map<string, string> prev;
    priority_queue<pair<int, string>> pq;

    dist[start] = 0;
    pq.push({ 0, start });


    while (!pq.empty()) {
        auto [cost, node] = pq.top(); pq.pop();
        cost = -cost;

        if (node == end) break;

        for (auto& [neighbor, weight] : graph[node]) {
            int newCost = cost + weight;
            if (!dist.count(neighbor) || newCost < dist[neighbor]) {
                dist[neighbor] = newCost;
                prev[neighbor] = node;
                pq.push({ -newCost, neighbor });
            }
        }
    }

    vector<string> path;
    for (string at = end; at != ""; at = prev[at]) {
        path.push_back(at);
        if (at == start) break;
    }
    reverse(path.begin(), path.end());
    return path;
}
