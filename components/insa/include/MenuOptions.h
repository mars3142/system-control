/**
 * @file MenuOptions.h
 * @brief Menu configuration structure and callback definitions
 * @details This header defines the menu_options_t structure which contains all
 *          necessary configuration options and callback functions for menu widgets.
 *          It provides the interface between menu widgets and the application's
 *          screen management system, display context, and input handling.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

// Standard libraries for function objects and smart pointers
#include <functional>
#include <memory>

// Project-specific headers
#include "common/Widget.h"
#include "../../persistence-manager/include/IPersistenceManager.h"
#include "u8g2.h"

class MenuItem;

/**
 * @struct menu_options_t
 * @brief Configuration structure for menu widgets containing display context and callbacks
 * @details This structure serves as a configuration container that provides menu widgets
 *          with access to the display system, screen management functions, input
 *          handling callbacks, and persistent storage.
 * 
 * @see Widget
 * @see ButtonType
 * @see IPersistenceManager
 */
typedef struct
{
    /**
     * @brief Pointer to u8g2 display context for graphics output operations
     */
    u8g2_t *u8g2;

    /**
     * @brief Callback function to set the current active screen
     */
    std::function<void(std::shared_ptr<Widget>)> setScreen;

    /**
     * @brief Callback function to add a new screen to the navigation stack
     */
    std::function<void(std::shared_ptr<Widget>)> pushScreen;

    /**
     * @brief Callback function to remove the top screen from the navigation stack
     */
    std::function<void()> popScreen;

    /**
     * @brief Callback function to handle button press events
     */
    std::function<void(ButtonType button)> onButtonClicked;

    /**
     * @brief Shared pointer to platform-independent persistence manager
     * @details This provides access to persistent key-value storage across different
     *          platforms. The actual implementation (SDL3 or ESP32/NVS) is determined
     *          at compile time based on the target platform.
     * 
     * @note The persistence manager is shared across all menu widgets and maintains
     *       its state throughout the application lifecycle.
     */
    std::shared_ptr<IPersistenceManager> persistenceManager;

} menu_options_t;