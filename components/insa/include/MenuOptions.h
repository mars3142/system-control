// Prevents multiple inclusions of this header file
#pragma once

// Standard libraries for function objects and smart pointers
#include <functional>
#include <memory>

// Project-specific headers
#include "common/Widget.h"
#include "u8g2.h"

// Structure for menu options and callback functions
typedef struct
{
    // Pointer to u8g2 display object for graphics output
    u8g2_t *u8g2;

    // Callback function to set the current screen
    // Parameter: Smart pointer to a Widget object
    std::function<void(std::shared_ptr<Widget>)> setScreen;

    // Callback function to add a new screen to the stack
    // Parameter: Smart pointer to a Widget object
    std::function<void(std::shared_ptr<Widget>)> pushScreen;

    // Callback function to remove the top screen from the stack
    // No parameters required
    std::function<void()> popScreen;

    // Callback function to handle button presses
    // Parameter: 8-bit button ID/status
    std::function<void(uint8_t button)> onButtonClicked;
} menu_options_t;