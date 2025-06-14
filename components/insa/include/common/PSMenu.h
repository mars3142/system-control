#pragma once

#include <functional>

#include "Common.h"
#include "MenuOptions.h"
#include "Widget.h"
#include "data/MenuItem.h"

/**
 * PSMenu - A menu widget class
 *
 * This class extends the Widget base class to provide a customizable menu system
 * with various types of interactive menu items including text buttons, selections,
 * number inputs, and toggles.
 */
class PSMenu : public Widget
{
public:
    /**
     * Constructor - Creates a new PSMenu instance
     * @param options Pointer to menu configuration options
     */
    explicit PSMenu(menu_options_t *options);

    /**
     * Destructor - Cleans up resources when menu is destroyed
     */
    ~PSMenu() override;

    /**
     * Adds a text-based menu item (button) to the menu
     * @param id Unique identifier for this menu item
     * @param text Display text shown on the menu
     * @param callback Function to call when this item is selected
     */
    void addText(uint8_t id, const std::string &text, const ButtonCallback &callback);

    /**
     * Adds a selection menu item (dropdown/list selection)
     * @param id Unique identifier for this menu item
     * @param text Display text/label for the selection
     * @param value Reference to current selected value (will be modified)
     * @param values Vector of all available options to choose from
     * @param callback Function to call when selection changes
     */
    void addSelection(uint8_t id, const std::string &text, std::string &value, const std::vector<std::string>& values,
                      const ButtonCallback &callback);

    /**
     * Adds a numeric input menu item
     * @param id Unique identifier for this menu item
     * @param text Display text/label for the number input
     * @param value Reference to current numeric value as string (will be modified)
     * @param callback Function to call when value changes
     */
    void addNumber(uint8_t id, const std::string &text, std::string &value, const ButtonCallback &callback);

    /**
     * Adds a toggle/checkbox menu item
     * @param id Unique identifier for this menu item
     * @param text Display text/label for the toggle
     * @param selected Current state of the toggle (true = on, false = off)
     * @param callback Function to call when toggle state changes
     */
    void addToggle(uint8_t id, const std::string &text, bool selected, const ButtonCallback &callback);

private:
    /**
     * Renders the entire menu on screen
     * Override from Widget base class
     */
    void render() override;

    /**
     * Handles button press events from the controller/input system
     * @param button The button that was pressed
     * Override from Widget base class
     */
    void onButtonClicked(uint8_t button) override;

    // Navigation event handlers
    /**
     * Handles down arrow/stick input - moves selection down
     */
    void onPressedDown();

    /**
     * Handles up arrow/stick input - moves selection up
     */
    void onPressedUp();

    /**
     * Handles left arrow/stick input - decreases value for current item
     */
    void onPressedLeft() const;

    /**
     * Handles right arrow/stick input - increases value for current item
     */
    void onPressedRight() const;

    /**
     * Handles select/confirm button (X on PlayStation controller)
     */
    void onPressedSelect() const;

    /**
     * Handles back/cancel button (Circle on PlayStation controller)
     */
    void onPressedBack() const;

    // Rendering helper methods
    /**
     * Draws the scroll bar indicating position in long menus
     */
    void drawScrollBar() const;

    /**
     * Draws the selection highlight box around current menu item
     */
    void drawSelectionBox() const;

    /**
     * Renders an individual menu item widget
     * @param item Pointer to the menu item to render
     * @param font Font to use for rendering text
     * @param x X coordinate for rendering position
     * @param y Y coordinate for rendering position
     */
    void renderWidget(const MenuItem *item, const uint8_t *font, int x, int y) const;

    // Member variables
    size_t m_selected_item = 0;        ///< Index of currently selected menu item
    std::vector<MenuItem> m_items;     ///< Collection of all menu items
    menu_options_t *m_options;         ///< Pointer to menu configuration options
};