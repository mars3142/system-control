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
#include "u8g2.h"

class MenuItem;

/**
 * @struct menu_options_t
 * @brief Configuration structure for menu widgets containing display context and callbacks
 * @details This structure serves as a configuration container that provides menu widgets
 *          with access to the display system, screen management functions, and input
 *          handling callbacks. It acts as the bridge between individual menu widgets
 *          and the broader application framework.
 * 
 * The structure contains:
 * - Display context for rendering operations
 * - Screen management callbacks for navigation
 * - Input handling callback for button events
 * 
 * All callback functions use std::function for type safety and flexibility,
 * allowing both function pointers and lambda expressions to be used.
 * 
 * @note This structure should be initialized by the application framework
 *       and passed to menu widgets during construction.
 * 
 * @see Widget
 * @see ButtonType
 */
typedef struct
{
    /**
     * @brief Pointer to u8g2 display context for graphics output operations
     * @details This pointer provides access to the u8g2 graphics library functions
     *          for rendering text, shapes, and other visual elements. It must be
     *          initialized and ready for drawing operations before being passed
     *          to menu widgets.
     * 
     * @note The menu widgets do not take ownership of this pointer and assume
     *       it remains valid throughout their lifetime. Ensure the u8g2 context
     *       is properly managed by the application framework.
     * 
     * @warning Must not be nullptr when passed to menu widgets
     */
    u8g2_t *u8g2;

    /**
     * @brief Callback function to set the current active screen
     * @param screen Smart pointer to the Widget that should become the active screen
     * 
     * @details This callback replaces the currently active screen with the provided
     *          widget. It is typically used for direct screen transitions where the
     *          previous screen should be completely replaced rather than stacked.
     * 
     * @note The callback takes ownership of the provided Widget through the shared_ptr.
     *       The previous screen will be destroyed unless other references exist.
     * 
     * @see pushScreen for adding screens to a navigation stack
     */
    std::function<void(std::shared_ptr<Widget>)> setScreen;

    /**
     * @brief Callback function to add a new screen to the navigation stack
     * @param screen Smart pointer to the Widget that should be pushed onto the screen stack
     * 
     * @details This callback adds a new screen on top of the current screen stack,
     *          allowing for hierarchical navigation where users can return to
     *          previous screens. Commonly used for sub-menus, settings screens,
     *          or modal dialogs.
     * 
     * @note The callback takes ownership of the provided Widget through the shared_ptr.
     *       The current screen remains in memory and can be returned to via popScreen().
     * 
     * @see popScreen for removing screens from the navigation stack
     * @see setScreen for direct screen replacement
     */
    std::function<void(std::shared_ptr<Widget>)> pushScreen;

    /**
     * @brief Callback function to remove the top screen from the navigation stack
     * @details This callback removes the currently active screen and returns to the
     *          previous screen in the navigation stack. It is typically used for
     *          "back" or "cancel" operations in hierarchical menu systems.
     * 
     * @note If the navigation stack is empty or contains only one screen, the
     *       behavior is implementation-dependent and should be handled gracefully
     *       by the application framework.
     * 
     * @see pushScreen for adding screens to the navigation stack
     */
    std::function<void()> popScreen;

    /**
     * @brief Callback function to handle button press events
     * @param button The type of button that was pressed
     * 
     * @details This callback is invoked when a button press event occurs that is
     *          not handled directly by the menu widget. It allows the application
     *          framework to implement global button handling logic, such as
     *          system-wide shortcuts or fallback behavior.
     * 
     * @note This callback is typically used for buttons that have application-wide
     *       meaning (e.g., home button, menu button) rather than widget-specific
     *       navigation which is handled internally by the widgets.
     * 
     * @see ButtonType for available button types
     * @see Widget::onButtonClicked for widget-specific button handling
     */
    std::function<void(ButtonType button)> onButtonClicked;
} menu_options_t;