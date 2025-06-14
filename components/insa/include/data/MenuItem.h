/**
 * @file MenuItem.h
 * @brief Menu item data structure for user interface menu systems
 * @details This header defines the MenuItem class which represents individual menu
 *          items that can be displayed and interacted with in user interface menus.
 *          It supports various types of menu items including simple buttons, toggles,
 *          value selectors, and multi-option selections with flexible callback handling.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "common/Common.h"

/**
 * @class MenuItem
 * @brief Flexible menu item class supporting various interaction types and behaviors
 * @details This class represents individual menu items that can be displayed in user
 *          interface menus. It provides a flexible foundation for different types of
 *          menu interactions including simple navigation buttons, toggle switches,
 *          value adjustments, and multi-option selections.
 * 
 * The MenuItem class supports multiple interaction patterns:
 * - **Simple Actions**: Basic menu items that execute a function when activated
 * - **Value Display**: Items that show a current value (read-only or editable)
 * - **Selection Lists**: Items that cycle through multiple predefined values
 * - **Toggle States**: Boolean items that can be switched on/off
 * - **Custom Behaviors**: Flexible callback system for specialized interactions
 * 
 * Each menu item is identified by a unique ID and has a type that defines its
 * visual appearance and interaction behavior. The callback system allows for
 * flexible event handling while maintaining type safety through std::function.
 * 
 * Key features include:
 * - Multiple constructor overloads for different menu item types
 * - Type-safe callback system with ButtonCallback function objects
 * - Support for both single values and value lists
 * - Flexible text and value management
 * - Efficient state management and validation
 * 
 * @note This class is designed to be lightweight and efficient for embedded
 *       systems while providing rich functionality for complex user interfaces.
 * 
 * @see ButtonCallback for callback function signature
 * @see ButtonType for available button types
 * @see Menu for menu container functionality
 */
class MenuItem
{
public:
    /**
     * @brief Constructs a simple action menu item with text and callback
     * @param id Unique identifier for this menu item within its parent menu
     * @param type Type identifier defining the item's behavior and visual appearance
     * @param text Display text shown to the user for this menu item
     * @param callback Function to call when the item is activated
     * 
     * @pre id must be unique within the parent menu context
     * @pre text should not be empty for proper user interface display
     * @pre callback should be a valid callable object
     * @post MenuItem is initialized as a simple action item ready for display
     * 
     * @details Creates a basic menu item that displays text and executes a callback
     *          when activated. This is the most common type of menu item used for
     *          navigation, simple actions, and command execution.
     * 
     * Typical use cases include:
     * - Navigation items (e.g., "Settings", "Back", "Exit")
     * - Action items (e.g., "Save", "Reset", "Start")
     * - Sub-menu entries (e.g., "Light Control", "System Info")
     * 
     * @note The callback is stored as a std::function and can be a lambda,
     *       function pointer, or any callable object matching the ButtonCallback signature.
     */
    MenuItem(uint8_t id, uint8_t type, std::string text, ButtonCallback callback);

    /**
     * @brief Constructs a value-displaying menu item with text, value, and callback
     * @param id Unique identifier for this menu item within its parent menu
     * @param type Type identifier defining the item's behavior and visual appearance
     * @param text Display text shown to the user for this menu item
     * @param value Current value associated with this item (displayed to user)
     * @param callback Function to call when the item is activated
     * 
     * @pre id must be unique within the parent menu context
     * @pre text should not be empty for proper user interface display
     * @pre callback should be a valid callable object
     * @post MenuItem is initialized with text and value display capabilities
     * 
     * @details Creates a menu item that displays both text and a current value.
     *          This type is commonly used for settings display, status information,
     *          or items where the current state needs to be visible to the user.
     * 
     * Typical use cases include:
     * - Setting displays (e.g., "Brightness: 75%")
     * - Status information (e.g., "Connection: WiFi")
     * - Editable values (e.g., "Timeout: 30s")
     * - Current selections (e.g., "Mode: Auto")
     * 
     * @note The value can be updated later using setValue() to reflect changes
     *       in the underlying system state.
     */
    MenuItem(uint8_t id, uint8_t type, std::string text, std::string value, ButtonCallback callback);

    /**
     * @brief Constructs a multi-selection menu item with selectable values
     * @param id Unique identifier for this menu item within its parent menu
     * @param type Type identifier defining the item's behavior and visual appearance
     * @param text Display text shown to the user for this menu item
     * @param values List of all available values that can be selected
     * @param index Currently selected value from the available options
     * @param callback Function to call when the item is activated
     * 
     * @pre id must be unique within the parent menu context
     * @pre text should not be empty for proper user interface display
     * @pre value should be present in the values vector
     * @pre values should not be empty and should contain valid options
     * @pre callback should be a valid callable object
     * @post MenuItem is initialized with multiple selectable values
     * 
     * @details Creates a menu item that allows selection from multiple predefined
     *          values. This type enables cycling through options or displaying
     *          selection dialogs, making it ideal for configuration settings
     *          with discrete choices.
     * 
     * Typical use cases include:
     * - Mode selection (e.g., "Display Mode: [Day, Night, Auto]")
     * - Configuration options (e.g., "Language: [English, Deutsch, Français]")
     * - Preset selection (e.g., "Profile: [Home, Office, Travel]")
     * - Format selection (e.g., "Time Format: [12H, 24H]")
     * 
     * @note The callback can implement cycling logic to move through the values
     *       or open a selection dialog for user choice.
     * @note The values vector is stored by copy, so modifications to the original
     *       vector after construction do not affect the menu item.
     */
    MenuItem(uint8_t id, uint8_t type, std::string text, std::vector<std::string> values, int index,
             ButtonCallback callback);

    /**
     * @brief Constructs a boolean/toggle menu item with on/off state
     * @param id Unique identifier for this menu item within its parent menu
     * @param type Type identifier defining the item's behavior and visual appearance
     * @param text Display text shown to the user for this menu item
     * @param selected Whether this item is currently selected/enabled/checked
     * @param callback Function to call when the item is activated
     * 
     * @pre id must be unique within the parent menu context
     * @pre text should not be empty for proper user interface display
     * @pre callback should be a valid callable object
     * @post MenuItem is initialized as a boolean toggle item
     * 
     * @details Creates a menu item that represents a boolean state (on/off, enabled/disabled,
     *          checked/unchecked). This type is ideal for settings that have binary states
     *          and need to show their current status visually.
     * 
     * Typical use cases include:
     * - Feature toggles (e.g., "Auto-save: ON")
     * - Enable/disable settings (e.g., "Sound: ENABLED")
     * - Checkbox-style options (e.g., "Show notifications: ✓")
     * - Boolean configurations (e.g., "Dark mode: OFF")
     * 
     * @note The selected state is converted to a string value internally for
     *       consistent value handling across all menu item types.
     * @note The callback typically implements toggle logic to switch between states.
     */
    MenuItem(uint8_t id, uint8_t type, std::string text, bool selected, ButtonCallback callback);

    /**
     * @brief Gets the unique identifier of this menu item
     * @return The menu item's unique ID as assigned during construction
     * 
     * @details Returns the unique identifier that distinguishes this menu item
     *          from others within the same menu context. This ID is used by
     *          menu systems for item identification, event routing, and state management.
     * 
     * @note The ID is immutable after construction and guaranteed to be unique
     *       within the menu context where this item is used.
     */
    [[nodiscard]] uint8_t getId() const;

    /**
     * @brief Gets the type identifier of this menu item
     * @return The menu item's type identifier defining its behavior
     * 
     * @details Returns the type identifier that defines how this menu item
     *          behaves and appears in the user interface. The type determines
     *          rendering style, interaction patterns, and event handling behavior.
     * 
     * @note The type is immutable after construction and should correspond
     *       to predefined menu item type constants defined in the system.
     */
    [[nodiscard]] uint8_t getType() const;

    /**
     * @brief Gets the display text of this menu item
     * @return Const reference to the menu item's display text
     * 
     * @details Returns the text that is displayed to the user for this menu item.
     *          This text serves as the primary label and should be descriptive
     *          enough for users to understand the item's purpose.
     * 
     * @note Returns a const reference for efficiency while preventing
     *       accidental modification of the text content.
     */
    [[nodiscard]] const std::string &getText() const;

    /**
     * @brief Gets the current value of this menu item
     * @return Const reference to the menu item's current value
     * 
     * @details Returns the current value associated with this menu item.
     *          For simple action items, this may be empty. For value-based
     *          items, this represents the current state, selection, or setting.
     * 
     * @note Returns a const reference for efficiency while preventing
     *       accidental modification through the getter.
     * @note For boolean items, the value is typically "true"/"false" or similar.
     */
    [[nodiscard]] const std::string &getValue() const;

    /**
     * @brief Sets a new value for this menu item
     * @param value The new value to assign to this menu item
     * 
     * @details Updates the current value of this menu item. This is commonly
     *          used to reflect changes in system state, user selections, or
     *          configuration updates. The new value should be appropriate
     *          for the menu item's type and purpose.
     * 
     * @note For multi-selection items, the value should be one of the
     *       predefined values in the values vector.
     * @note For boolean items, typical values are "true"/"false", "ON"/"OFF",
     *       or other boolean representations.
     * @note The value update does not automatically trigger the callback;
     *       this is purely for state management.
     */
    void setValue(const std::string &value);

    /**
     * @brief Handles button press events for this menu item
     * @param button The type of button that was pressed
     * 
     * @details Processes button press events by invoking the associated callback
     *          function if one exists. This method serves as the event handler
     *          that connects user interactions to the menu item's functionality.
     * 
     * The method performs the following actions:
     * - Validates that the ID matches this menu item
     * - Checks if a callback function is available
     * - Invokes the callback with the provided button type
     * - Handles any callback-related error conditions gracefully
     * 
     * @note This method is typically called by the parent menu system when
     *       user interaction occurs on this menu item.
     * @note If no callback is set, the method returns without error.
     * @note The callback is responsible for implementing the specific behavior
     *       for the button press (navigation, value changes, actions, etc.).
     */
    void onButtonPressed(ButtonType button) const;

    /**
     * @brief Checks if this menu item has an associated callback function
     * @return true if a callback function is set, false otherwise
     * 
     * @details Determines whether this menu item has a valid callback function
     *          that can be invoked when the item is activated. This is useful
     *          for menu systems that need to distinguish between interactive
     *          and non-interactive items.
     * 
     * @note Menu items without callbacks are typically used for display-only
     *       purposes such as headers, separators, or status information.
     * @note Interactive menu items should always have callbacks to provide
     *       meaningful user interaction.
     */
    [[nodiscard]] bool hasCallback() const;

    [[nodiscard]] int getIndex() const;

    std::vector<std::string> getValues() const;

    [[nodiscard]] size_t getItemCount() const;

    [[nodiscard]] MenuItem copyWith(size_t index) const;

private:
    /**
     * @brief Unique identifier for this menu item
     * @details Stores the unique ID that distinguishes this menu item from others
     *          within the same menu context. Used for event routing and item identification.
     */
    uint8_t m_id;

    /**
     * @brief Type identifier defining the item's behavior and appearance
     * @details Stores the type that determines how this menu item behaves,
     *          how it's rendered, and what interaction patterns it supports.
     */
    uint8_t m_type;

    /**
     * @brief Display text shown to the user
     * @details Stores the primary text label that is displayed to users,
     *          serving as the main identifier and description of the menu item's purpose.
     */
    std::string m_text;

    /**
     * @brief Current value associated with this menu item
     * @details Stores the current value for value-based menu items, representing
     *          the current state, selection, or setting that should be displayed to the user.
     */
    std::string m_value;

    /**
     * @brief Available values for selection-based menu items
     * @details Stores the list of all possible values that can be selected for
     *          multi-option menu items. Used for cycling through options or
     *          displaying selection dialogs.
     */
    std::vector<std::string> m_values;

    int m_index = -1;

    /**
     * @brief Callback function invoked when the menu item is activated
     * @details Stores the function object that implements the menu item's behavior
     *          when activated by user interaction. Uses std::function for flexibility
     *          and type safety.
     */
    ButtonCallback m_callback;
};