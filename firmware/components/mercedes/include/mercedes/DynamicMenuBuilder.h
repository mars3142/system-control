#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

/**
 * @brief Callback for dynamic menu events
 * @param id The String ID of the menu item interacted with
 * @param actionTopic The optional MQTT topic or action identifier from the JSON
 * @param value The new value (e.g. "true" for toggle, "2" for selection)
 */
using MenuActionCallback =
    std::function<void(const std::string &id, const std::string &actionTopic, const std::string &value)>;

/**
 * @brief Provider callback to fetch real-time state from NVS/OpenThread
 * @param id The String ID of the menu item
 * @return The current state as string (e.g. "true", "false", or a selection value), or empty string to use JSON
 * defaults
 */
using ItemValueProvider = std::function<std::string(const std::string &id)>;

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
    bool toggleValue = false;
    std::vector<MenuSelectionItemDef> selectionItems;
};

struct MenuScreenDef
{
    std::string id;
    std::string title;
    std::vector<MenuItemDef> items;
};

/**
 * @class DynamicMenuBuilder
 * @brief A helper class to construct Menus from a JSON payload
 */
class DynamicMenuBuilder
{
  public:
    /**
     * @brief Get the singleton instance of the DynamicMenuBuilder
     */
    static DynamicMenuBuilder &getInstance();

    // Delete copy and move constructors to enforce singleton pattern
    DynamicMenuBuilder(const DynamicMenuBuilder &) = delete;
    void operator=(const DynamicMenuBuilder &) = delete;
    ~DynamicMenuBuilder() = default;

    /**
     * @brief Parses a JSON string and populates the target Menu
     * @param jsonPayload The JSON string containing menu definitions
     * @return true if successfully parsed and built, false otherwise
     */
    bool buildFromJson(const std::string &jsonPayload);

    /**
     * @brief Sets the callback to be executed when a dynamic item is triggered
     */
    void setActionCallback(MenuActionCallback callback);

    /**
     * @brief Sets the provider to fetch real-time states for rendering
     */
    void setItemValueProvider(ItemValueProvider provider);

    /**
     * @brief Pseudo-rendering: Outputs the current menu to the log
     */
    void render();

    /**
     * @brief Simulates pressing an item via String ID
     */
    void handleItemPress(const std::string &itemId);

  private:
    DynamicMenuBuilder(); // Private constructor

    MenuActionCallback m_actionCallback; // Optional, in case you need callbacks in addition to the ActionManager
    ItemValueProvider m_valueProvider;   // Fetches real states from external sources (NVS/OpenThread)

    std::map<std::string, MenuScreenDef> m_screens;
    std::string m_currentScreenId;
};
