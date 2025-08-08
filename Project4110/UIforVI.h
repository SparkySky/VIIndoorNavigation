#pragma once
#include <vector>
#include <string>

class UIforVI {
public:
    UIforVI();    
    
    std::vector<std::string> destinations;  // Array storing available destination
    int curIndex;   // Used for Scroll or selection
    std::string selectDestination();

    void narrateCurrentOption();
    char getUserInput();
    std::string selectDestination(std::string curLocation);
};