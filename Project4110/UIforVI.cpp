#include "UIforVI.h"
#include "Text2Speech.h"
#include <iostream>
#include <string>

Text2Speech narrator;

UIforVI::UIforVI() {
    // Location - Can loaded from config or database
    destinations = { "N001", "N002", "N003", "N004", "N005", "N006", "N007", "N008", "N009", "N010" };
    curIndex = 0;
}

void UIforVI::narrateCurrentOption() {
    
    narrator.speak("Option: " + destinations[curIndex]);
}

char UIforVI::getUserInput() {
    std::cout << "[u] Scroll Up | [d] Scroll Down | [c] Confirm\n> ";
    char input;
    std::cin >> input;
    return input;
}

std::string UIforVI::selectDestination(std::string curLocation) {
    narrator.speak("Select Destination");
    if (destinations[curIndex] == curLocation) 
        curIndex = (curIndex++ + destinations.size()) % destinations.size();  // Skip to next if selection == location.
    narrateCurrentOption();

    while (true) {
        char input = getUserInput();
        if (input == 'u') {
            curIndex = (--curIndex + destinations.size()) % destinations.size();
            if (destinations[curIndex] == curLocation) curIndex--;
        }
        else if (input == 'd') {
            curIndex = (++curIndex) % destinations.size();
            if (destinations[curIndex] == curLocation) curIndex++;
        }
        else if (input == 'c') {
            Text2Speech narrator;
            narrator.speak("Destination selected: " + destinations[curIndex]);
            return destinations[curIndex];
        }
        narrateCurrentOption();
    }
}
