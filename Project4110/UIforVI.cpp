#include "UIForVI.h"
#include <fstream>
#include <sstream>

using namespace std;

UIForVI::UIForVI() {
    destinations.push_back("Cancel"); // Always include "Cancel" as first option
    loadDestinationsFromFile("NavigationFile\\NodeMapping.txt"); // Load all options from NodeMapping.txt
    active = false;
    selected_index = 0;
}

bool UIForVI::validateDestination(const string& input) {
    for (const auto& dest : destinations) {
        if (dest == input) {
            return 1;
        }
    }
    return 0;
}

// Load nodes from NodeMapping.txt
void UIForVI::loadDestinationsFromFile(const string& filename) {
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        size_t pos = line.find(' ');
        if (pos != string::npos) {
            destinations.push_back(line.substr(0, pos)); // Only take the part before ':'
        }
    }
}

// If node scanned, activate this node
void UIForVI::activate(const std::string& start_node) {
    active = true;
    origin_node = start_node;
    selected_index = 0; // Default to "Cancel"
    draw_to_console(); // Draw the initial menu
}

// If cancelled OR navigation end, set deactivate the node, 
// so user can start from other node
void UIForVI::deactivate() {
    active = false;
}

bool UIForVI::is_active() const {
    return active;
}

// Helper: call to move to previous selection
void UIForVI::scroll_up() {
    if (!active) return;
    selected_index = (selected_index - 1 + destinations.size()) % destinations.size();
    if (destinations[selected_index] == origin_node) scroll_up(); // Skip origin
    draw_to_console();
}

// Helper: call to move to next selection
void UIForVI::scroll_down() {
    if (!active) return;
    selected_index = (selected_index + 1) % destinations.size();
    if (destinations[selected_index] == origin_node) scroll_down(); // Skip origin
    draw_to_console();
}

// Get the current node
std::string UIForVI::get_current_selection() const {
    if (!active) return "";
    return destinations[selected_index];
}


// Simple UI
void UIForVI::draw_to_console() const {
    if (!active) return;
    system("cls"); // Clear the console
    cout << "--- DESTINATION SELECTION ---" << endl;
    cout << "  Origin: " << origin_node << endl;
    cout << "-----------------------------" << endl;
    cout << "  (w/s) or (Mouse Scroll)" << endl;
    cout << "  (c) or (Left Click)" << endl;
    cout << "-----------------------------" << endl;
    for (int i = 0; i < destinations.size(); ++i) {
        if (destinations[i] == origin_node) continue; // Don't show origin in list
        cout << (i == selected_index ? " > " : "   ") << destinations[i] << endl;
    }
    cout << "-----------------------------" << endl;
}