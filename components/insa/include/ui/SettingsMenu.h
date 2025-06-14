/**
 * @file SettingsMenu.h
 * @brief Settings menu implementation for application configuration
 * @details This header defines the SettingsMenu class which provides a user interface
 *          for configuring application settings and preferences. It extends the Menu
 *          base class to offer a specialized settings management interface with
 *          various configuration options and system parameters.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "common/Menu.h"

/**
 * @class SettingsMenu
 * @brief Settings menu interface class for application configuration management
 * @details This final class inherits from Menu and provides a comprehensive settings
 *          interface for the application. It allows users to configure various aspects
 *          of the system including display preferences, system behavior, network
 *          settings, and other configurable parameters.
 * 
 * The SettingsMenu class extends the base Menu functionality by:
 * - Providing settings-specific menu items (toggles, selections, number inputs)
 * - Managing configuration persistence and validation
 * - Organizing settings into logical categories and sections
 * - Implementing real-time preview of setting changes where applicable
 * 
 * Typical settings categories include:
 * - Display settings (brightness, contrast, theme)
 * - System preferences (timeouts, auto-save, etc.)
 * - Network configuration (if applicable)
 * - User interface preferences
 * - Hardware-specific parameters
 * 
 * @note This class is marked as final and cannot be inherited from.
 * @note Settings changes are typically applied immediately or after confirmation,
 *       depending on the specific setting type and system requirements.
 * 
 * @see Menu for base menu functionality
 * @see menu_options_t for configuration structure
 * @see MainMenu for navigation back to main interface
 */
class SettingsMenu final : public Menu
{
public:
    /**
     * @brief Constructs the settings menu with the specified configuration
     * @param options Pointer to menu options configuration structure
     * 
     * @pre options must not be nullptr and must remain valid for the menu's lifetime
     * @pre options->u8g2 must be initialized and ready for graphics operations
     * @pre All callback functions in options must be properly configured
     * @post SettingsMenu is initialized with all available configuration options and ready for use
     * 
     * @details The constructor initializes the settings menu by creating all the
     *          configuration menu items based on the current system state and
     *          available options. This includes:
     *          - Loading current setting values from persistent storage
     *          - Creating appropriate menu items (toggles, selections, number inputs)
     *          - Setting up validation ranges and allowed values
     *          - Organizing items in a logical, user-friendly order
     * 
     * The menu automatically detects which settings are available based on
     * hardware capabilities and system configuration, ensuring only relevant
     * options are presented to the user.
     * 
     * @note The menu does not take ownership of the options structure and assumes
     *       it remains valid throughout the menu's lifetime.
     * @note Setting values are loaded from persistent storage during construction
     *       and changes are typically saved automatically or on confirmation.
     * 
     * @see Menu::Menu for base class construction details
     */
    explicit SettingsMenu(menu_options_t *options);
};