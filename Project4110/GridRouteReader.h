#pragma once

#include <string>
#include <vector>

using namespace std;

class GridRouteReader {
public:
    vector<string> gridLoader(const string& filename);

private:
    vector<string> grid;
};
