#include <iostream>
#include <string>

#include "UIforVI.h"
#include "Text2Speech.h"

using namespace std;

Text2Speech narrate;

UIForVI::UIForVI() {
    // Available Location -> Can load from config later
    destinations = { "Cancel", "N001", "N002", "N003", "N004", "N005", "N006", "N007", "N008A", "N008B", "N009", "N010", "N011", "N012"};
    curIndex = 0;
}

string UIForVI::selectDestination(string curLocation) {
    curIndex = 0; // Reset to 'Cancel'
    narrate.speak_low_priority("Select Destination");

    // Function below is needed if first destinations is a real place. Else no need
    //if (destinations[curIndex] == curLocation) // Pre check if simillar location and dest
    //    curIndex = (++curIndex + destinations.size()) % destinations.size();  // Skip to next if selection == location.

    narrateCurrentOption(); // Narrate current position

    while (true) { // User input: W - Previous, S - Next, C- Comfirm
        char input = getUserInput();
        if (input == 'w') { // Scroll up
            curIndex = (--curIndex + destinations.size()) % destinations.size();
            if (destinations[curIndex] == curLocation) 
                curIndex = (--curIndex + destinations.size()) % destinations.size(); // Post check
        }
        else if (input == 's') { /// Scroll down
            curIndex = (++curIndex) % destinations.size();
            if (destinations[curIndex] == curLocation) 
                curIndex = (++curIndex + destinations.size()) % destinations.size(); // Post check
        }
        else if (input == 'c') {
            if (curIndex == 0) {
                narrate.speak_low_priority("Navigation Cancelled");
                return "";
            }
            narrate.speak_low_priority("Destination selected: " + destinations[curIndex]);
            return destinations[curIndex];
        }
        narrateCurrentOption();
    }
}

/*Helper*/
// Narrate current option
void UIForVI::narrateCurrentOption() {
    narrate.speak_low_priority("Option: " + destinations[curIndex]);
}

// Get user input
char UIForVI::getUserInput() {
    cout << "[w] Scroll Up | [s] Scroll Down | [c] Confirm\n> ";
    char input;
    cin >> input;
    return input;
}