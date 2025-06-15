#pragma once

#include "common/Menu.h"

/**
 * @class LightSettingsMenu
 * @brief Menu for configuring light system settings including sections and LED parameters
 * @details This menu extends the base Menu class to provide specialized functionality
 *          for managing light system configurations. It handles dynamic section management
 *          and provides persistence for user settings.
 */
class LightSettingsMenu final : public Menu
{
public:
    /**
     * @brief Constructs a LightSettingsMenu with the specified options
     * @param options Pointer to menu configuration options structure
     * @details Initializes the menu with section counter and default section settings
     */
    explicit LightSettingsMenu(menu_options_t *options);

private:
    /**
     * @brief Handles button press events for light settings menu items
     * @param menuItem The menu item that received the button press
     * @param button The type of button that was pressed
     * @details Manages value switching, dynamic section list updates, and
     *          persistence of section values when settings are modified
     */
    void onButtonPressed(const MenuItem& menuItem, ButtonType button) override;

    menu_options_t *m_options; ///< Pointer to menu configuration options
};