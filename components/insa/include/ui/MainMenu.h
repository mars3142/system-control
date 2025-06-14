#pragma once

#include "common/PSMenu.h"

/**
 * MainMenu class - represents the main menu interface of the application
 * Inherits from PSMenu to provide menu functionality
 */
class MainMenu final : public PSMenu
{
public:
    /**
     * Constructor - initializes the main menu with given options
     * @param options Pointer to menu options configuration
     */
    explicit MainMenu(menu_options_t *options);

private:
    /**
     * Handles button press events from the menu interface
     * @param id Button identifier that was pressed
     * @param button Type of button that was pressed
     */
    void onButtonPressed(uint8_t id, ButtonType button) const;

    /**
     * Pointer to menu options configuration
     * Stores the menu configuration passed during construction
     */
    menu_options_t *m_options;
};