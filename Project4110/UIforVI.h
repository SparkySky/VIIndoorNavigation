#pragma once
#include <vector>
#include <string>

class UIForVI {
public:
    UIForVI(); 
    
    std::vector<std::string> destinations;   // Array storing available destination
    int curIndex;                       // Used for Scroll or selection

    void narrateCurrentOption();
    char getUserInput();
    std::string selectDestination(std::string curLocation);
};