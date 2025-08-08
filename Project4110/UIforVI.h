#pragma once
#include <vector>
#include <string>

using namespace std;

class UIForVI {
public:
    UIForVI(); 
    
    vector<std::string> destinations;   // Array storing available destination
    int curIndex;                       // Used for Scroll or selection

    void narrateCurrentOption();
    char getUserInput();
    string selectDestination(string curLocation);
    string selectDestination();
};