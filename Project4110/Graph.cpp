#include "Graph.h"
#include <queue>
#include <vector>
#include <limits>
#include <algorithm>

// Constructor
Graph::Graph(int V) {
    this->V = V;
    this->adj = new std::list<AdjListNode>[V];
}

// Destructor
Graph::~Graph() {
    delete[] adj;
}

// <<< ADD THIS COPY CONSTRUCTOR >>>
// Performs a deep copy of the graph
Graph::Graph(const Graph& other) {
    V = other.V;
    adj = new std::list<AdjListNode>[V];
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
    adj = new std::list<AdjListNode>[V];
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

// Implementation of Dijkstra's algorithm (const corrected)
std::vector<int> Graph::dijkstra(int src, int dest) const {
    if (src >= V || dest >= V) return {}; // Safety check

    // A priority queue to store vertices that are being preprocessed.
    // The first value in pair is the distance, the second is the vertex label.
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>> pq;

    // Vector for distances and initialize all distances as infinite
    std::vector<int> dist(V, std::numeric_limits<int>::max());

    // Vector to store the path predecessor
    std::vector<int> pred(V, -1);

    // Insert source itself in priority queue and initialize its distance as 0.
    pq.push(std::make_pair(0, src));
    dist[src] = 0;

    while (!pq.empty()) {
        // The first vertex in pair is the minimum distance vertex.
        int u = pq.top().second;
        int dist_u = pq.top().first;
        pq.pop();

        // --- CORRECTED LOGIC ---
        // If the distance we just extracted is already greater than a known shorter path,
        // then this is a stale entry in the queue. Skip it.
        if (dist_u > dist[u]) {
            continue;
        }

        // If we have reached the destination, we can stop.
        if (u == dest) {
            break;
        }

        // 'i' is used to get all adjacent vertices of a vertex
        for (auto const& i : adj[u]) {
            int v = i.dest;
            int weight = i.weight;

            // If there is shorter path to v through u.
            if (dist[v] > dist[u] + weight) {
                // Updating distance of v
                dist[v] = dist[u] + weight;
                pq.push(std::make_pair(dist[v], v));
                pred[v] = u; // Set the predecessor of v as u
            }
        }
    }

    // Reconstruct the path from destination to source
    std::vector<int> path;
    int crawl = dest;

    // Check if a path was found (pred[dest] will be set if dest is reachable)
    if (pred[crawl] == -1 && crawl != src) {
        return {}; // No path found
    }

    while (crawl != -1) {
        path.push_back(crawl);
        crawl = pred[crawl];
    }

    std::reverse(path.begin(), path.end());

    if (!path.empty() && path[0] == src) {
        return path;
    }

    return {};
}