#pragma once

#include <functional>

// Enumeration defining the different types of buttons available in the system
// NONE represents no button pressed or an invalid button state
enum class ButtonType { 
    NONE,     // No button or invalid state
    UP,       // Up directional button
    DOWN,     // Down directional button  
    LEFT,     // Left directional button
    RIGHT,    // Right directional button
    SELECT,   // Select/confirm button
    BACK      // Back/cancel button
};

// Type alias for button event callback function
// Parameters:
//   - uint8_t: Button identifier or additional data
//   - ButtonType: The type of button that was pressed
typedef std::function<void(uint8_t, ButtonType)> ButtonCallback;