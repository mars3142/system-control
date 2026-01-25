/**
 * @file LightMenu.h
 * @brief Light control menu implementation for lighting system management
 * @details This header defines the LightMenu class which provides a specialized
 *          user interface for controlling lighting systems and illumination features.
 *          It extends the Menu base class to offer comprehensive light control
 *          functionality including brightness adjustment, color selection, and
 *          lighting mode configuration.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "common/Menu.h"

/**
 * @class LightMenu
 * @brief Light control menu interface class for illumination system management
 * @details This final class inherits from Menu and provides a comprehensive interface
 *          for controlling various aspects of the lighting system. It allows users to
 *          adjust brightness levels, select colors, configure lighting modes, and
 *          manage automated lighting behaviors.
 *
 * The LightMenu class extends the base Menu functionality by:
 * - Providing light-specific control options (brightness, color, modes)
 * - Implementing real-time lighting preview and feedback
 * - Managing lighting presets and custom configurations
 * - Handling immediate lighting adjustments and scheduled operations
 *
 * Typical lighting control features include:
 * - Brightness adjustment (0-100% or similar range)
 * - Color selection (RGB, HSV, or predefined colors)
 * - Lighting modes (solid, fade, strobe, custom patterns)
 * - Timer-based automation (on/off schedules)
 * - Preset management (save/load favorite configurations)
 * - Zone-based control (if multiple light zones are supported)
 *
 * The menu provides immediate visual feedback by applying changes to the
 * connected lighting hardware in real-time as users navigate through options.
 *
 * @note This class is marked as final and cannot be inherited from.
 * @note Lighting changes are typically applied immediately for instant feedback,
 *       with the option to save configurations as presets.
 *
 * @see Menu for base menu functionality
 * @see menu_options_t for configuration structure
 * @see MainMenu for navigation back to main interface
 */
class LightMenu final : public Menu
{
  public:
    /**
     * @brief Constructs the light control menu with the specified configuration
     * @param options Pointer to menu options configuration structure
     *
     * @pre options must not be nullptr and must remain valid for the menu's lifetime
     * @pre options->u8g2 must be initialized and ready for graphics operations
     * @pre All callback functions in options must be properly configured
     * @pre Lighting hardware must be initialized and responsive
     * @post LightMenu is initialized with current lighting state and ready for user interaction
     *
     * @details The constructor initializes the light control menu by:
     *          - Reading current lighting system state and parameters
     *          - Creating appropriate menu items for available lighting features
     *          - Setting up real-time preview capabilities
     *          - Loading saved lighting presets and configurations
     *          - Configuring value ranges and validation for lighting parameters
     *
     * The menu automatically detects available lighting capabilities and presents
     * only the controls that are supported by the connected hardware. This ensures
     * a consistent user experience across different lighting system configurations.
     *
     * @note The menu does not take ownership of the options structure and assumes
     *       it remains valid throughout the menu's lifetime.
     * @note Current lighting state is preserved and can be restored if the user
     *       exits without saving changes.
     *
     * @see Menu::Menu for base class construction details
     */
    explicit LightMenu(menu_options_t *options);

    const char *getName() const override;

  private:
    /**
     * @brief Handles button press events specific to light control menu items
     * @param menuItem
     * @param button Type of button that was pressed
     *
     * @details Overrides the base Menu class method to provide light control-specific
     *          button handling logic. This method processes user interactions with
     *          lighting control items and performs appropriate actions such as:
     *          - Adjusting brightness levels (increment/decrement)
     *          - Changing color values or selecting predefined colors
     *          - Switching between lighting modes and patterns
     *          - Saving/loading lighting presets
     *          - Toggling lighting zones on/off
     *          - Applying lighting changes immediately to hardware
     *
     * The method provides real-time feedback by immediately applying lighting
     * changes to the connected hardware, allowing users to see the effects of
     * their adjustments instantly. It also handles validation to ensure that
     * lighting parameters remain within safe and supported ranges.
     *
     * Special handling includes:
     * - Smooth transitions for brightness adjustments
     * - Color wheel navigation for color selection
     * - Mode cycling for lighting patterns
     * - Confirmation prompts for preset operations
     *
     * @note This method is called by the base Menu class when a button press
     *       occurs on a menu item, after the base class has handled standard
     *       navigation operations.
     * @note Changes are applied immediately to provide instant visual feedback,
     *       but can be reverted if the user cancels or exits without saving.
     *
     * @see Menu::onButtonPressed for the base implementation
     * @see ButtonType for available button types
     */
    void onButtonPressed(const MenuItem &menuItem, ButtonType button) override;

    void onMessageReceived(const message_t *msg);

    /**
     * @brief Pointer to menu options configuration structure
     * @details Stores a reference to the menu configuration passed during construction.
     *          This pointer provides access to the display context and callback functions
     *          needed for menu operations, screen transitions, and lighting control
     *          communication with the hardware subsystem.
     *
     * The configuration enables:
     * - Display context for rendering lighting control interface
     * - Screen management callbacks for navigation to other menus
     * - Hardware communication for real-time lighting control
     * - System callbacks for lighting state persistence
     *
     * @note This pointer is not owned by the LightMenu and must remain valid
     *       throughout the menu's lifetime. It is managed by the application framework.
     *
     * @warning Must not be modified after construction as it may be shared
     *          with other components and contains critical system callbacks.
     */
    menu_options_t *m_options;
};