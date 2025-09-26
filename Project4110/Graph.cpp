#include "Graph.h"
#include <queue>
#include <vector>
#include <limits>
#include <algorithm>
#include <opencv2/core/types.hpp>

using namespace cv;
using namespace std;

// Constructor
Graph::Graph(int V) {
    this->V = V;
    this->adj = new list<AdjListNode>[V];
}

// Destructor
Graph::~Graph() {
    delete[] adj;
}

// <<< ADD THIS COPY CONSTRUCTOR >>>
// Performs a deep copy of the graph
Graph::Graph(const Graph& other) {
    V = other.V;
    adj = new list<AdjListNode>[V];
    for (int i = 0; i < V; ++i) {
        adj[i] = other.adj[i];
    }
}

// <<< ADD THIS COPY ASSIGNMENT OPERATOR >>>
// Handles assignment (e.g., graph1 = graph2)
Graph& Graph::operator=(const Graph& other) {
    if (this == &other) { // Handle self-assignment
        return *this;
    }

    // Clean up existing resource
    delete[] adj;

    // Copy data from other object
    V = other.V;
    adj = new list<AdjListNode>[V];
    for (int i = 0; i < V; ++i) {
        adj[i] = other.adj[i];
    }

    return *this;
}


// Adds an edge from src to dest.
void Graph::addEdge(int src, int dest, int weight) {
    // Check for out-of-bounds access
    if (src >= V || dest >= V) return;

    AdjListNode srcNode = { dest, weight };
    adj[src].push_back(srcNode);

    AdjListNode destNode = { src, weight };
    adj[dest].push_back(destNode);
}

vector<int> Graph::astar(int src, int dest, int gridWidth) const {
    if (src >= V || dest >= V) return {};

    auto indexToPoint = [gridWidth](int idx) {
        return cv::Point(idx % gridWidth, idx / gridWidth);
    };

    auto manhattan = [&](int a, int b) {
        cv::Point pa = indexToPoint(a);
        cv::Point pb = indexToPoint(b);
        return abs(pa.x - pb.x) + abs(pa.y - pb.y);
    };

    using PQElem = pair<int, int>; // (f_score, node)
    priority_queue<PQElem, vector<PQElem>, greater<PQElem>> openSet;

    vector<int> gScore(V, numeric_limits<int>::max());
    vector<int> fScore(V, numeric_limits<int>::max());
    vector<int> pred(V, -1);

    gScore[src] = 0;
    fScore[src] = manhattan(src, dest);
    openSet.push({fScore[src], src});

    while (!openSet.empty()) {
        int current = openSet.top().second;
        openSet.pop();

        if (current == dest) break;

        for (const auto& neighbor : adj[current]) {
            int tentative_gScore = gScore[current] + neighbor.weight;
            if (tentative_gScore < gScore[neighbor.dest]) {
                pred[neighbor.dest] = current;
                gScore[neighbor.dest] = tentative_gScore;
                fScore[neighbor.dest] = tentative_gScore + manhattan(neighbor.dest, dest);
                openSet.push({fScore[neighbor.dest], neighbor.dest});
            }
        }
    }

    vector<int> path;
    int crawl = dest;
    if (pred[crawl] == -1 && crawl != src) return {};
    while (crawl != -1) {
        path.push_back(crawl);
        crawl = pred[crawl];
    }
    reverse(path.begin(), path.end());
    if (!path.empty() && path[0] == src) return path;
    return {};
}
