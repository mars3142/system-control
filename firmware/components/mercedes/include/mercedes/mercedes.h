#pragma once

#include "heimdall/button_type.h"

#include <functional>
#include <map>
#include <stack>
#include <string>
#include <vector>

/**
 * @brief Callback for dynamic menu events
 * @param id The string ID of the menu item interacted with
 * @param actionTopic The optional MQTT topic or action identifier from the JSON
 * @param value The new value (e.g. "true" for toggle, "2" for selection)
 */
using MenuActionCallback =
    std::function<void(const std::string &id, const std::string &actionTopic, const std::string &value)>;

/**
 * @brief Provider callback to fetch real-time state from NVS/OpenThread
 * @param id The string ID of the menu item
 * @return The current state as string, or empty string to use JSON defaults
 */
using ItemValueProvider = std::function<void(const std::string &id, char *buf, size_t bufSize)>;

/**
 * @brief Callback notified when menu state changes (screen switch, selection move, value change)
 */
using MenuStateChangedCallback = std::function<void()>;

/**
 * @brief Called when navigating to a screen with dynamic=true.
 *
 * The provider should call addOrReplaceScreen() to populate or refresh the
 * screen's items (and any sub-screens it references) before navigation
 * completes. The callback runs synchronously inside navigateToScreen(), so
 * it must not block indefinitely.
 *
 * @param screenId  The id of the screen being entered.
 */
using DynamicScreenProvider = std::function<void(const std::string &screenId)>;

struct MenuSelectionItemDef
{
    std::string value;
    std::string label;
};

struct MenuItemDef
{
    std::string id;
    std::string type;
    std::string label;
    std::string actionTopic;
    std::string targetScreenId;
    bool persistent = false;
    std::string valueType; // "string" (default) or "int" or "bool" — controls NVS read/write type
    bool toggleValue = false;
    int selectionIndex = 0;
    std::vector<MenuSelectionItemDef> selectionItems;
    std::string visibleWhenItemId;
    std::string visibleWhenValue;
};

struct MenuScreenDef
{
    std::string id;
    std::string title;
    std::vector<MenuItemDef> items;
    bool dynamic = false; /**< If true, DynamicScreenProvider is called before navigation completes */
};

/**
 * @class Mercedes
 * @brief Data model and navigation logic for JSON-driven dynamic menus
 */
class Mercedes
{
  public:
    static Mercedes &getInstance();

    Mercedes(const Mercedes &) = delete;
    void operator=(const Mercedes &) = delete;
    ~Mercedes() = default;

    /**
     * @brief Parses a JSON string and populates the menu structure
     */
    bool buildFromJson(const std::string &jsonPayload);

    /**
     * @brief Handles button input for navigation and item interaction
     */
    void handleInput(button_type_t button);

    /**
     * @brief Sets the callback executed when a dynamic item is triggered
     */
    void setActionCallback(MenuActionCallback callback);

    /**
     * @brief Sets the provider to fetch real-time states for rendering
     */
    void setItemValueProvider(ItemValueProvider provider);

    /**
     * @brief Sets the callback notified when menu state changes (for rendering)
     */
    void setStateChangedCallback(MenuStateChangedCallback callback);

    /**
     * @brief Sets the provider called when navigating to a screen with dynamic=true.
     *
     * The provider receives the target screen id and should call
     * addOrReplaceScreen() to populate it before the navigation completes.
     */
    void setDynamicScreenProvider(DynamicScreenProvider provider);

    /**
     * @brief Insert or replace a screen in the internal screen map.
     *
     * Used by the DynamicScreenProvider to inject runtime-generated screens
     * (e.g. per-device capability screens) that are not present in menu.json.
     * If this is the currently displayed screen, triggers stateChangedCallback.
     */
    void addOrReplaceScreen(const MenuScreenDef &screen);

    /**
     * @brief Add an item to an existing screen if no item with the same id exists.
     *
     * Used by dynamic providers to append runtime items (e.g. multicast toggle)
     * to a screen without replacing items that already have in-memory state.
     * No-op if the screen does not exist or the item id is already present.
     */
    void ensureItemInScreen(const std::string &screenId, const MenuItemDef &item);

    /**
     * @brief Remove an item from a screen by id.
     *
     * No-op if the screen or item does not exist.
     */
    void removeItemFromScreen(const std::string &screenId, const std::string &itemId);

    // --- State accessors (used by hermes for rendering) ---

    /**
     * @brief Returns the current screen definition, or nullptr if none
     */
    const MenuScreenDef *getCurrentScreen() const;

    /**
     * @brief Returns the currently selected item index
     */
    size_t getSelectedIndex() const;

    /**
     * @brief Returns true if there are screens on the back-stack
     */
    bool canGoBack() const;

    /**
     * @brief Returns the item value provider (for rendering dynamic values)
     */
    const ItemValueProvider *getItemValueProvider() const;

    /**
     * @brief Returns true if the item should be visible given current menu state
     */
    bool isItemVisible(const MenuItemDef &item) const;

    /**
     * @brief Updates an item's in-memory state by id and string value — triggers stateChangedCallback
     */
    void updateItemValue(const std::string &id, const std::string &value);

  private:
    Mercedes();

    void navigateToScreen(const std::string &screenId);
    void activateCurrentItem();
    void adjustCurrentItem(button_type_t button);

    MenuActionCallback m_actionCallback;
    ItemValueProvider m_valueProvider;
    MenuStateChangedCallback m_stateChangedCallback;
    DynamicScreenProvider m_dynamicScreenProvider;

    std::map<std::string, MenuScreenDef> m_screens;
    std::string m_currentScreenId;
    size_t m_selectedIndex = 0;
    std::stack<std::pair<std::string, size_t>> m_screenHistory;
};
