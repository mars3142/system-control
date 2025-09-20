/**
 * @file MainMenu.h
 * @brief Main menu implementation for the application's primary interface
 * @details This header defines the MainMenu class which serves as the primary
 *          user interface entry point for the application. It extends the Menu
 *          base class to provide a customized main menu experience with
 *          application-specific menu items and navigation behavior.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "common/Menu.h"

/**
 * @class MainMenu
 * @brief Main menu interface class providing the primary application navigation
 * @details This final class inherits from Menu and represents the main menu interface
 *          of the application. It serves as the primary entry point for user interaction
 *          and provides navigation to all major application features and sub-menus.
 *
 * The MainMenu class customizes the base Menu functionality by:
 * - Defining application-specific menu items during construction
 * - Implementing custom button handling for main menu navigation
 * - Managing transitions to sub-menus and application features
 *
 * This class is typically the first screen presented to users after the splash
 * screen and serves as the central hub for all application functionality.
 *
 * @note This class is marked as final and cannot be inherited from.
 *
 * @see Menu for base menu functionality
 * @see menu_options_t for configuration structure
 */
class MainMenu final : public Menu
{
  public:
    /**
     * @brief Constructs the main menu with the specified configuration
     * @param options Pointer to menu options configuration structure
     *
     * @pre options must not be nullptr and must remain valid for the menu's lifetime
     * @pre options->u8g2 must be initialized and ready for graphics operations
     * @pre All callback functions in options must be properly configured
     * @post MainMenu is initialized with application-specific menu items and ready for use
     *
     * @details The constructor initializes the main menu by setting up all the
     *          primary application menu items such as:
     *          - Settings access
     *          - Feature-specific menus
     *          - System controls
     *          - Application onExit options
     *
     * @note The menu does not take ownership of the options structure and assumes
     *       it remains valid throughout the menu's lifetime.
     */
    explicit MainMenu(menu_options_t *options);

  private:
    /**
     * @brief Handles button press events specific to main menu items
     * @param menuItem
     * @param button Type of button that was pressed
     *
     * @details Overrides the base Menu class method to provide main menu-specific
     *          button handling logic. This method processes user interactions with
     *          main menu items and initiates appropriate actions such as:
     *          - Navigation to sub-menus (Settings, Light Control, etc.)
     *          - Direct feature activation
     *          - System operations
     *
     * The method uses the menu item ID to determine which action to take and
     * utilizes the callback functions provided in m_options to perform screen
     * transitions or other application-level operations.
     *
     * @note This method is called by the base Menu class when a button press
     *       occurs on a menu item, after the base class has handled standard
     *       navigation operations.
     *
     * @see Menu::onButtonPressed for the base implementation
     * @see ButtonType for available button types
     */
    void onButtonPressed(const MenuItem &menuItem, ButtonType button) override;

    /**
     * @brief Pointer to menu options configuration structure
     * @details Stores a reference to the menu configuration passed during construction.
     *          This pointer provides access to the display context and callback functions
     *          needed for menu operations, screen transitions, and user interaction handling.
     *
     * The configuration includes:
     * - Display context for rendering operations
     * - Screen management callbacks for navigation
     * - Input handling callbacks for button events
     *
     * @note This pointer is not owned by the MainMenu and must remain valid
     *       throughout the menu's lifetime. It is managed by the application framework.
     *
     * @warning Must not be modified after construction as it may be shared
     *          with other components.
     */
    menu_options_t *m_options;
};