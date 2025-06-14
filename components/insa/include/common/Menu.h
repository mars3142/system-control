/**
 * @file Menu.h
 * @brief Menu widget class for creating interactive menu systems
 * @details This header defines the Menu class which extends the Widget base class
 *          to provide a comprehensive, customizable menu system supporting various
 *          types of interactive menu items including text buttons, selections,
 *          number inputs, and toggles. The menu supports navigation with directional
 *          input and provides visual feedback through selection highlighting and scrollbars.
 * @author System Control Team
 * @date 2025-06-14
 */

#pragma once

#include "Common.h"
#include "MenuOptions.h"
#include "Widget.h"
#include "data/MenuItem.h"

/**
 * @class Menu
 * @brief A comprehensive menu widget class for interactive user interfaces
 * @details This class extends the Widget base class to provide a customizable menu system
 *          with support for various types of interactive menu items. It handles user input
 *          through directional navigation and action buttons, provides visual feedback
 *          through selection highlighting, and supports scrolling for long menu lists.
 * 
 * The menu system supports four types of menu items:
 * - Text buttons: Simple selectable text items
 * - Selection items: Dropdown/list selection with multiple options
 * - Number inputs: Numeric value adjustment controls
 * - Toggle items: Boolean on/off switches
 * 
 * @note Menu items are identified by unique IDs and can be dynamically added
 *       after menu creation.
 * 
 * @see Widget
 * @see MenuItem
 * @see menu_options_t
 */
class Menu : public Widget
{
public:
    /**
     * @brief Constructs a new Menu instance with the specified configuration
     * @param options Pointer to menu configuration options structure
     * 
     * @pre options must not be nullptr and must remain valid for the menu's lifetime
     * @post Menu is initialized with the provided configuration and ready for item addition
     * 
     * @note The menu does not take ownership of the options structure and assumes
     *       it remains valid throughout the menu's lifetime.
     */
    explicit Menu(menu_options_t *options);

    /**
     * @brief Destructor - Cleans up resources when menu is destroyed
     * @details Properly releases any allocated resources and ensures clean shutdown
     *          of the menu system.
     */
    ~Menu() override;

    /**
     * @brief Adds a text-based menu item (button) to the menu
     * @param id Unique identifier for this menu item (must be unique within the menu)
     * @param text Display text shown on the menu item
     * 
     * @pre id must be unique within this menu instance
     * @post A new text menu item is added to the menu's item collection
     * 
     * @note Text items act as buttons and generate selection events when activated
     */
    void addText(uint8_t id, const std::string &text);

    /**
     * @brief Adds a selection menu item (dropdown/list selection) to the menu
     * @param id Unique identifier for this menu item (must be unique within the menu)
     * @param text Display text/label for the selection item
     * @param values Vector of all available options to choose from
     * @param index Reference to current selected value (will be modified by user interaction)
     *
     * @pre id must be unique within this menu instance
     * @pre values vector must not be empty
     * @pre value must be one of the strings in the values vector
     * @post A new selection menu item is added with the specified options
     * 
     * @note The value parameter is modified directly when the user changes the selection
     */
    void addSelection(uint8_t id, const std::string &text, const std::vector<std::string> &values, int index);

    /**
     * @brief Adds a toggle/checkbox menu item to the menu
     * @param id Unique identifier for this menu item (must be unique within the menu)
     * @param text Display text/label for the toggle item
     * @param selected Current state of the toggle (true = on/enabled, false = off/disabled)
     * 
     * @pre id must be unique within this menu instance
     * @post A new toggle menu item is added with the specified initial state
     * 
     * @note Toggle state can be changed through user interaction with select button
     */
    void addToggle(uint8_t id, const std::string &text, bool selected);

protected:
    /**
     * @brief Virtual callback method for handling button press events on specific menu items
     * @param item The menu item that received the button press
     * @param button The type of button that was pressed
     * 
     * @details This method can be overridden by derived classes to implement custom
     *          button handling logic for specific menu items. The base implementation
     *          is empty, allowing derived classes to selectively handle events.
     * 
     * @note Override this method in derived classes to implement custom menu item
     *       interaction behavior beyond the standard navigation and value modification.
     */
    virtual void onButtonPressed(const MenuItem &item, const ButtonType button)
    {
        // Base implementation intentionally empty - override in derived classes as needed
    }

    MenuItem getItem(int index);

    [[nodiscard]] size_t getItemCount() const;

    void setItemSize(size_t size);

    void replaceItem(int index, const MenuItem &item);

private:
    /**
     * @brief Renders the entire menu on screen
     * @details Override from Widget base class. Handles the complete rendering process
     *          including menu items, selection highlighting, and scroll indicators.
     * 
     * @note This method is called during each frame's render cycle
     */
    void render() override;

    /**
     * @brief Handles button press events from the input system
     * @param button The button that was pressed
     * @details Override from Widget base class. Processes user input and delegates
     *          to appropriate handler methods based on button type.
     * 
     * @see ButtonType for available button types
     */
    void onButtonClicked(ButtonType button) override;

    // Navigation event handlers
    /**
     * @brief Handles down arrow/stick input - moves selection down in the menu
     * @details Moves the current selection to the next menu item, wrapping to the
     *          beginning if at the end of the list.
     */
    void onPressedDown();

    /**
     * @brief Handles up arrow/stick input - moves selection up in the menu
     * @details Moves the current selection to the previous menu item, wrapping to the
     *          end if at the beginning of the list.
     */
    void onPressedUp();

    /**
     * @brief Handles left arrow/stick input - decreases value for current item
     * @details For selection items: moves to previous option in the list
     *          For number items: decreases the numeric value
     *          For other items: no action
     */
    void onPressedLeft() const;

    /**
     * @brief Handles right arrow/stick input - increases value for current item
     * @details For selection items: moves to next option in the list
     *          For number items: increases the numeric value
     *          For other items: no action
     */
    void onPressedRight() const;

    /**
     * @brief Handles select/confirm button press
     * @details Activates the currently selected menu item:
     *          - Text items: triggers selection event
     *          - Toggle items: toggles the boolean state
     *          - Other items: context-dependent behavior
     */
    void onPressedSelect() const;

    /**
     * @brief Handles back/cancel button press
     * @details Typically used to exit the menu or return to a previous screen.
     *          The specific behavior depends on the menu configuration.
     */
    void onPressedBack() const;

    // Rendering helper methods
    /**
     * @brief Draws the scroll bar indicating position in long menus
     * @details Renders a visual scroll indicator when the menu contains more items
     *          than can be displayed on screen simultaneously.
     */
    void drawScrollBar() const;

    /**
     * @brief Draws the selection highlight box around current menu item
     * @details Renders visual feedback showing which menu item is currently selected
     *          and will respond to user input.
     */
    void drawSelectionBox() const;

    /**
     * @brief Renders an individual menu item widget
     * @param item Pointer to the menu item to render (must not be nullptr)
     * @param font Font to use for rendering text
     * @param x X coordinate for rendering position
     * @param y Y coordinate for rendering position
     * 
     * @pre item must not be nullptr
     * @pre font must be a valid u8g2 font
     * @pre x and y must be valid screen coordinates
     * 
     * @details Handles the rendering of a single menu item based on its type,
     *          including text, current values, and any type-specific visual elements.
     */
    void renderWidget(const MenuItem *item, const uint8_t *font, int x, int y) const;

    // Member variables
    size_t m_selected_item = 0; ///< Index of currently selected menu item (0-based)
    std::vector<MenuItem> m_items; ///< Collection of all menu items in display order
    menu_options_t *m_options; ///< Pointer to menu configuration options (not owned)
};