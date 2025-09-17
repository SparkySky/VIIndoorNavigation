#pragma once

#include <string>
#include <vector>
#include "Graph.h" // Include the Graph definition

class GridRouteReader {
public:
    GridRouteReader(); // Add a constructor
    bool gridLoader(const std::string& filename); // Changed to bool for success/failure

    const Graph& getGraph() const; // Getter for the graph
    int getGridWidth() const;      // Getter for the grid width

    // Inside public section of GridRouteReader class
    const std::vector<std::string>& getGrid() const;
private:
    std::vector<std::string> grid;
    Graph graph; // The graph representation of the grid
    int gridWidth; // To help convert coordinates to index
};
