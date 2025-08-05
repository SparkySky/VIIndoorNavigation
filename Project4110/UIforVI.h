#pragma once
#include <vector>
#include <string>

class UIforVI {
public:
    UIforVI();
    std::string selectDestination();

private:
    std::vector<std::string> destinations;
    int currentIndex;

    void narrateCurrentOption();
    char getUserInput();
};