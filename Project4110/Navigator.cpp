#include "Navigator.h"
#include <queue>
#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

void Navigator::loadMap(const std::string& filename) {
    std::ifstream file(filename);
    json j;
    file >> j;
    for (auto& node : j.items()) {
        std::string id = node.key();
        for (auto& neighbor : node.value()) {
            graph[id].emplace_back(neighbor["id"], neighbor["cost"]);
        }
    }
}

std::vector<std::string> Navigator::findPath(const std::string& start, const std::string& end) {
    std::unordered_map<std::string, int> dist;
    std::unordered_map<std::string, std::string> prev;
    std::priority_queue<std::pair<int, std::string>> pq;

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

    std::vector<std::string> path;
    for (std::string at = end; at != ""; at = prev[at]) {
        path.push_back(at);
        if (at == start) break;
    }
    std::reverse(path.begin(), path.end());
    return path;
}