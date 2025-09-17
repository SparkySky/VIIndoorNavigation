#pragma once

#include <vector>
#include <list>

// A structure to represent a node in the adjacency list
struct AdjListNode {
    int dest;
    int weight;
};

// A structure to represent a graph. A graph is an array of adjacency lists.
// Size of array will be V (number of vertices in graph)
class Graph {
private:
    int V; // Number of vertices
    std::list<AdjListNode>* adj; // Pointer to an array containing adjacency lists

public:
    // Constructor: Initializes a graph with V vertices.
    // We are providing a default constructor and an initializer
    Graph(int V = 0);

    // Destructor
    ~Graph();

    Graph(const Graph& other); // Copy Constructor
    Graph& operator=(const Graph& other); // Copy Assignment Operator

    // Function to add an edge to graph
    void addEdge(int src, int dest, int weight = 1);

    // Dijkstra's algorithm to find the shortest path from src to dest
    // Returns a vector of integers representing the path
    std::vector<int> dijkstra(int src, int dest) const;
};