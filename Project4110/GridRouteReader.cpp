#include "GridRouteReader.h"
#include <fstream>
#include <iostream>
#include <sstream>

// Constructor: Initializes the GridRouteReader object.
GridRouteReader::GridRouteReader() {}

// gridLoader: Reads a text file, builds a graph from it, and stores the grid.
bool GridRouteReader::gridLoader(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open grid file: " << filename << std::endl;
        return false;
    }

    std::string line;
    std::vector<std::vector<int>> tempGrid;
    int rows = 0;
    int cols = 0;

    // First pass: Read the grid into a temporary 2D vector
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        grid.push_back(line); // Store the string representation for other uses if needed

        std::stringstream ss(line);
        int val;
        std::vector<int> rowVec;
        cols = 0;
        while (ss >> val) {
            rowVec.push_back(val);
            cols++;
        }
        tempGrid.push_back(rowVec);
        rows++;
    }
    file.close();

    if (rows == 0 || cols == 0) {
        std::cerr << "Error: Grid is empty or invalid." << std::endl;
        return false;
    }

    // Second pass: Build the graph from the 2D grid
    this->graph = Graph(rows * cols); // Initialize graph with total number of cells
    this->gridWidth = cols; // Store grid width

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // If the current cell is a path (0), add edges to neighbors
            if (tempGrid[r][c] == 0) {
                int currentNode = r * cols + c;

                // Check neighbor above
                if (r > 0 && tempGrid[r - 1][c] == 0) {
                    int neighborNode = (r - 1) * cols + c;
                    graph.addEdge(currentNode, neighborNode);
                }
                // Check neighbor below
                if (r < rows - 1 && tempGrid[r + 1][c] == 0) {
                    int neighborNode = (r + 1) * cols + c;
                    graph.addEdge(currentNode, neighborNode);
                }
                // Check neighbor left
                if (c > 0 && tempGrid[r][c - 1] == 0) {
                    int neighborNode = r * cols + (c - 1);
                    graph.addEdge(currentNode, neighborNode);
                }
                // Check neighbor right
                if (c < cols - 1 && tempGrid[r][c + 1] == 0) {
                    int neighborNode = r * cols + (c + 1);
                    graph.addEdge(currentNode, neighborNode);
                }
            }
        }
    }
    std::cout << "Grid map loaded and graph built successfully." << std::endl;
    return true;
}

// getGraph: Returns a constant reference to the graph object.
const Graph& GridRouteReader::getGraph() const {
    return graph;
}

// getGridWidth: Returns the width of the loaded grid.
int GridRouteReader::getGridWidth() const {
    return gridWidth;
}

const std::vector<std::string>& GridRouteReader::getGrid() const {
    return grid;
}
