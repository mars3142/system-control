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

    void addTextCounter(uint8_t id, const std::string &text, const uint8_t value);

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

    /**
     * @brief Retrieves a menu item by its index position
     * @param index Zero-based index of the menu item to retrieve
     * @return MenuItem object at the specified index
     *
     * @pre index must be within valid range [0, getItemCount()-1]
     * @post Returns a copy of the menu item at the specified position
     *
     * @throws std::out_of_range if index is invalid
     *
     * @note This method returns a copy of the menu item, not a reference
     */
    MenuItem getItem(int index);

    /**
     * @brief Gets the total number of menu items in the menu
     * @return Size of the menu items collection
     *
     * @post Returns current count of menu items (>= 0)
     *
     * @note This count includes all types of menu items (text, selection, toggle)
     */
    [[nodiscard]] size_t getItemCount() const;

    /**
     * @brief Dynamically adjusts the number of menu items to the specified size
     * @param size Target number of menu items the menu should contain
     *
     * @details If the target size is larger than current item count, new selection
     *          items are added using the first item's values as template. If the
     *          target size is smaller, excess items are removed from the end.
     *
     * @pre size must be > 0 and at least one menu item must exist as template
     * @post Menu contains exactly 'size' number of items
     *
     * @note New items are created as selection items with auto-generated names
     *       in the format "Section X" where X is the item number
     */
    void setItemSize(size_t size, int8_t startIndex = 0);

    /**
     * @brief Toggles the boolean state of a toggle menu item
     * @param menuItem The toggle menu item whose state should be flipped
     *
     * @pre menuItem must be of type TOGGLE
     * @post The menu item's value is switched between "true" and "false"
     *
     * @details Changes "true" to "false" and "false" to "true" for toggle items.
     *          The modified item replaces the original in the menu's item collection.
     *
     * @note This method directly modifies the menu's internal state
     */
    void toggle(const MenuItem &menuItem);

    /**
     * @brief Setzt den Zustand eines Toggle-Menüeintrags explizit
     * @param menuItem Der zu ändernde Toggle-Menüeintrag
     * @param state Neuer Zustand (true = aktiviert, false = deaktiviert)
     *
     * @pre menuItem muss vom Typ TOGGLE sein
     * @post Der Wert des Menüeintrags wird auf den angegebenen Zustand gesetzt
     *
     * @details Diese Methode setzt den Wert eines Toggle-Menüeintrags gezielt auf den gewünschten Zustand.
     *          Der geänderte Eintrag ersetzt das Original in der internen Menüstruktur.
     *
     * @note Diese Methode verändert direkt den internen Zustand des Menüs.
     */
    void setToggle(const MenuItem &menuItem, const bool state);

    /**
     * @brief Changes the selected value of a selection menu item based on button input
     * @param menuItem The selection menu item to modify
     * @param button The directional button pressed (LEFT or RIGHT)
     *
     * @pre menuItem must be of type SELECTION with valid values array
     * @post The menu item's selected index is adjusted based on button direction
     *
     * @details LEFT button moves to previous option (wraps to end if at beginning),
     *          RIGHT button moves to next option (wraps to beginning if at end).
     *          Other button types are ignored.
     *
     * @note The modified item replaces the original in the menu's item collection
     */
    MenuItem switchValue(const MenuItem &menuItem, ButtonType button);

    void setSelectionIndex(const MenuItem &menuItem, int index);

  private:
    MenuItem replaceItem(int index, const MenuItem &item);

    void Render() override;

    void OnButtonClicked(ButtonType button) override;

    void onPressedDown();

    void onPressedUp();

    void onPressedLeft() const;

    void onPressedRight() const;

    void onPressedSelect() const;

    void onPressedBack() const;

    void drawScrollBar() const;

    void drawSelectionBox() const;

    void renderWidget(const MenuItem *item, const uint8_t *font, int x, int y) const;

    // Member variables
    size_t m_selected_item = 0;
    std::vector<MenuItem> m_items;
    menu_options_t *m_options;
};