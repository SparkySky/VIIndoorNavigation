#pragma once
#include <iostream>
#include <vector>
#include <string>

class UIForVI {
public:
    UIForVI();

    // --- State Management ---
    void activate(const std::string& start_node);
    void deactivate();
    bool is_active() const;

    // --- Input Handling ---
    void scroll_up();
    void scroll_down();
    std::string get_current_selection() const;

    // --- Drawing ---
    void draw_to_console() const;

private:
    bool active;
    int selected_index;
    std::string origin_node;
    std::vector<std::string> destinations;
};