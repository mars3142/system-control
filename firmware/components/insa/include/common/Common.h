/**
 * @file Common.h
 * @brief Common definitions and types for the INSA component
 * @details This header file contains shared enumerations, type definitions, and 
 *          callback function types used throughout the INSA component system.
 *          It provides the foundation for button handling and event management.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include <functional>

/**
 * @enum ButtonType
 * @brief Enumeration defining the different types of buttons available in the system
 * @details This enumeration represents all possible button types that can be handled
 *          by the system's input management. NONE represents no button pressed or 
 *          an invalid button state, while the other values correspond to specific
 *          directional and action buttons.
 */
enum class ButtonType
{
    NONE, ///< No button pressed or invalid button state
    UP, ///< Up directional button for navigation
    DOWN, ///< Down directional button for navigation
    LEFT, ///< Left directional button for navigation
    RIGHT, ///< Right directional button for navigation
    SELECT, ///< Select/confirm button for accepting choices
    BACK ///< Back/cancel button for returning to previous state
};

// Forward declaration of MenuItem to avoid circular dependency
class MenuItem;

/**
 * @typedef ButtonCallback
 * @brief Type alias for button event callback function
 * @details This function type is used to define callback functions that handle
 *          button press events. The callback receives information about which
 *          button was pressed and any additional context data.
 * 
 * @param MenuItem menu item for the specific action
 * @param ButtonType The type of button that was pressed
 * 
 * @note The first parameter can be used to distinguish between multiple instances
 *       of the same button type or to pass additional event-specific data.
 * 
 * @example
 * @code
 * ButtonCallback myCallback = [](const MenuItem& item, ButtonType type) {
 *     if (type == ButtonType::SELECT) {
 *         // Handle select button press
 *         processSelection(item);
 *     }
 * };
 * @endcode
 */
typedef std::function<void(MenuItem, ButtonType)> ButtonCallback;

// Include MenuItem.h after the typedef to avoid circular dependency
#include "data/MenuItem.h"