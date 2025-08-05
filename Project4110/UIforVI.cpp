#include "UIforVI.h"
#include "Text2Speech.h"
#include <iostream>

UIforVI::UIforVI() {
    // Example destinations; can be loaded from config or database
    destinations = { "Reception", "Toilet", "Lift", "Room A101", "Exit" };
    currentIndex = 0;
}

void UIforVI::narrateCurrentOption() {
    Text2Speech narrator;
    narrator.speak("Option: " + destinations[currentIndex]);
}

char UIforVI::getUserInput() {
    std::cout << "[u] Scroll Up | [d] Scroll Down | [c] Confirm\n> ";
    char input;
    std::cin >> input;
    return input;
}

std::string UIforVI::selectDestination() {
    narrateCurrentOption();

    while (true) {
        char input = getUserInput();

        if (input == 'u') {
            currentIndex = (currentIndex - 1 + destinations.size()) % destinations.size();
            narrateCurrentOption();
        }
        else if (input == 'd') {
            currentIndex = (currentIndex + 1) % destinations.size();
            narrateCurrentOption();
        }
        else if (input == 'c') {
            Text2Speech narrator;
            narrator.speak("Destination selected: " + destinations[currentIndex]);
            return destinations[currentIndex];
        }
    }
}
