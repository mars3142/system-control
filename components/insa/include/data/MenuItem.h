#pragma once

#include <functional>
#include <string>

#include "common/Common.h"

/**
 * Represents a menu item that can be displayed in a user interface.
 * Supports different types of menu items with various interaction capabilities.
 */
class MenuItem
{
public:
    /**
     * Constructor for a simple menu item with text and callback.
     * @param id Unique identifier for this menu item
     * @param type Type of the menu item (defines behavior and appearance)
     * @param text Display text for the menu item
     * @param callback Function to call when the item is activated
     */
    MenuItem(uint8_t id, uint8_t type, std::string text, ButtonCallback callback);
    
    /**
     * Constructor for a menu item with a value field.
     * @param id Unique identifier for this menu item
     * @param type Type of the menu item
     * @param text Display text for the menu item
     * @param value Current value associated with this item
     * @param callback Function to call when the item is activated
     */
    MenuItem(uint8_t id, uint8_t type, std::string text, std::string value, ButtonCallback callback);
    
    /**
     * Constructor for a menu item with multiple selectable values.
     * @param id Unique identifier for this menu item
     * @param type Type of the menu item
     * @param text Display text for the menu item
     * @param value Currently selected value
     * @param values List of all available values for selection
     * @param callback Function to call when the item is activated
     */
    MenuItem(uint8_t id, uint8_t type, std::string text, std::string value, std::vector<std::string> values,
             ButtonCallback callback);
    
    /**
     * Constructor for a boolean/toggle menu item.
     * @param id Unique identifier for this menu item
     * @param type Type of the menu item
     * @param text Display text for the menu item
     * @param selected Whether this item is currently selected/checked
     * @param callback Function to call when the item is activated
     */
    MenuItem(uint8_t id, uint8_t type, std::string text, bool selected, ButtonCallback callback);
    
    /**
     * Gets the unique identifier of this menu item.
     * @return The menu item's ID
     */
    [[nodiscard]] uint8_t getId() const;
    
    /**
     * Gets the type of this menu item.
     * @return The menu item's type
     */
    [[nodiscard]] uint8_t getType() const;
    
    /**
     * Gets the display text of this menu item.
     * @return Reference to the menu item's text
     */
    [[nodiscard]] const std::string &getText() const;
    
    /**
     * Gets the current value of this menu item.
     * @return Reference to the menu item's current value
     */
    [[nodiscard]] const std::string &getValue() const;
    
    /**
     * Sets a new value for this menu item.
     * @param value The new value to set
     */
    void setValue(const std::string &value);
    
    /**
     * Handles button press events for this menu item.
     * Calls the associated callback function if one exists.
     * @param id The ID of the item that was pressed
     * @param button The type of button that was pressed
     */
    void onButtonPressed(uint8_t id, ButtonType button) const;
    
    /**
     * Checks if this menu item has an associated callback function.
     * @return true if a callback is set, false otherwise
     */
    [[nodiscard]] bool hasCallback() const;

private:
    uint8_t m_id;                           // Unique identifier for this menu item
    uint8_t m_type;                         // Type defining the item's behavior
    std::string m_text;                     // Display text shown to the user
    std::string m_value;                    // Current value (for value-based items)
    std::vector<std::string> m_values;      // Available values (for selection items)
    ButtonCallback m_callback;              // Function to call when activated
};