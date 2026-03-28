#include "mercedes/DynamicMenuBuilder.h"
#include "heimdall/action_manager.h"
#include <cJSON.h>
#include <esp_log.h>

static const char *TAG = "DynamicMenu";

DynamicMenuBuilder &DynamicMenuBuilder::getInstance()
{
    static DynamicMenuBuilder instance;
    return instance;
}

DynamicMenuBuilder::DynamicMenuBuilder()
{
}

void DynamicMenuBuilder::setActionCallback(MenuActionCallback callback)
{
    m_actionCallback = callback;
}

void DynamicMenuBuilder::setItemValueProvider(ItemValueProvider provider)
{
    m_valueProvider = provider;
}

bool DynamicMenuBuilder::buildFromJson(const std::string &jsonPayload)
{
    // Parse JSON
    cJSON *root = cJSON_Parse(jsonPayload.c_str());
    if (!root)
    {
        ESP_LOGE(TAG, "Error parsing JSON payload");
        return false;
    }

    // Reset previous state
    m_screens.clear();
    m_currentScreenId = "";

    cJSON *screens = cJSON_GetObjectItem(root, "screens");
    if (screens && cJSON_IsArray(screens))
    {
        int numScreens = cJSON_GetArraySize(screens);

        for (int i = 0; i < numScreens; i++)
        {
            cJSON *screenItem = cJSON_GetArrayItem(screens, i);
            if (!screenItem)
                continue;

            MenuScreenDef screenDef;
            cJSON *screenId = cJSON_GetObjectItem(screenItem, "id");
            cJSON *screenTitle = cJSON_GetObjectItem(screenItem, "title");

            if (screenId && cJSON_IsString(screenId))
                screenDef.id = screenId->valuestring;
            if (screenTitle && cJSON_IsString(screenTitle))
                screenDef.title = screenTitle->valuestring;

            // Set the first screen as the start screen
            if (m_currentScreenId.empty() && !screenDef.id.empty())
            {
                m_currentScreenId = screenDef.id;
            }

            cJSON *items = cJSON_GetObjectItem(screenItem, "items");
            if (items && cJSON_IsArray(items))
            {
                int numItems = cJSON_GetArraySize(items);
                for (int j = 0; j < numItems; j++)
                {
                    cJSON *item = cJSON_GetArrayItem(items, j);
                    if (!item)
                        continue;

                    MenuItemDef itemDef;
                    cJSON *idItem = cJSON_GetObjectItem(item, "id");
                    cJSON *typeItem = cJSON_GetObjectItem(item, "type");
                    cJSON *labelItem = cJSON_GetObjectItem(item, "label");
                    cJSON *actionItem = cJSON_GetObjectItem(item, "actionTopic");
                    cJSON *targetItem = cJSON_GetObjectItem(item, "targetScreenId");
                    cJSON *persistentItem = cJSON_GetObjectItem(item, "persistent");

                    if (idItem && cJSON_IsString(idItem))
                        itemDef.id = idItem->valuestring;
                    if (typeItem && cJSON_IsString(typeItem))
                        itemDef.type = typeItem->valuestring;
                    if (labelItem && cJSON_IsString(labelItem))
                        itemDef.label = labelItem->valuestring;
                    if (actionItem && cJSON_IsString(actionItem))
                        itemDef.actionTopic = actionItem->valuestring;
                    if (targetItem && cJSON_IsString(targetItem))
                        itemDef.targetScreenId = targetItem->valuestring;
                    if (persistentItem && cJSON_IsBool(persistentItem))
                        itemDef.persistent = cJSON_IsTrue(persistentItem);

                    cJSON *valueItem = cJSON_GetObjectItem(item, "value");
                    if (valueItem && cJSON_IsBool(valueItem))
                        itemDef.toggleValue = cJSON_IsTrue(valueItem);

                    // Parse sub-items for 'selection' type
                    if (itemDef.type == "selection")
                    {
                        cJSON *selectionItems = cJSON_GetObjectItem(item, "items");
                        if (selectionItems && cJSON_IsArray(selectionItems))
                        {
                            int numSelItems = cJSON_GetArraySize(selectionItems);
                            for (int k = 0; k < numSelItems; k++)
                            {
                                cJSON *selItem = cJSON_GetArrayItem(selectionItems, k);
                                if (!selItem)
                                    continue;

                                MenuSelectionItemDef selDef;
                                cJSON *valItem = cJSON_GetObjectItem(selItem, "value");
                                cJSON *lblItem = cJSON_GetObjectItem(selItem, "label");

                                if (valItem && cJSON_IsString(valItem))
                                    selDef.value = valItem->valuestring;
                                if (lblItem && cJSON_IsString(lblItem))
                                    selDef.label = lblItem->valuestring;

                                itemDef.selectionItems.push_back(selDef);
                            }
                        }
                    }

                    screenDef.items.push_back(itemDef);
                }
            }
            m_screens[screenDef.id] = screenDef;
        }
    }

    // Free RAM (IMPORTANT for cJSON!)
    cJSON_Delete(root);
    return true;
}

void DynamicMenuBuilder::render()
{
    if (m_currentScreenId.empty() || m_screens.find(m_currentScreenId) == m_screens.end())
    {
        ESP_LOGE(TAG, "No active screen found to render.");
        return;
    }

    const MenuScreenDef &screen = m_screens[m_currentScreenId];

    ESP_LOGI(TAG, "===================================");
    ESP_LOGI(TAG, " SCREEN: %s", screen.title.c_str());
    ESP_LOGI(TAG, "-----------------------------------");

    for (size_t i = 0; i < screen.items.size(); i++)
    {
        const auto &item = screen.items[i];
        std::string persistentStr = item.persistent ? " [Persistent]" : "";

        // Fetch real state from NVS/OpenThread if available, otherwise use JSON default
        std::string externalState = m_valueProvider ? m_valueProvider(item.id) : "";

        if (item.type == "submenu")
        {
            ESP_LOGI(TAG, " - %s%s (-> opens '%s')", item.label.c_str(), persistentStr.c_str(),
                     item.targetScreenId.c_str());
        }
        else if (item.type == "action")
        {
            ESP_LOGI(TAG, " - %s%s [Action: %s]", item.label.c_str(), persistentStr.c_str(), item.actionTopic.c_str());
        }
        else if (item.type == "toggle")
        {
            bool isOn = item.toggleValue;
            if (!externalState.empty())
                isOn = (externalState == "true");

            ESP_LOGI(TAG, " - %s%s [Toggle: %s]", item.label.c_str(), persistentStr.c_str(), isOn ? "ON" : "OFF");
        }
        else if (item.type == "selection")
        {
            ESP_LOGI(TAG, " - %s%s [Selection]:", item.label.c_str(), persistentStr.c_str());
            for (const auto &sel : item.selectionItems)
            {
                bool isSelected = (!externalState.empty() && externalState == sel.value);
                ESP_LOGI(TAG, "     %s %s (Value: %s)", isSelected ? "[x]" : "[ ]", sel.label.c_str(),
                         sel.value.c_str());
            }
        }
        else
        {
            ESP_LOGI(TAG, " - %s%s", item.label.c_str(), persistentStr.c_str());
        }
    }
    ESP_LOGI(TAG, "===================================");
}

void DynamicMenuBuilder::handleItemPress(const std::string &itemId)
{
    if (m_currentScreenId.empty() || m_screens.find(m_currentScreenId) == m_screens.end())
    {
        return;
    }

    std::string mainItemId = itemId;
    std::string subValue;
    size_t separatorPos = itemId.find(':');
    if (separatorPos != std::string::npos)
    {
        mainItemId = itemId.substr(0, separatorPos);
        subValue = itemId.substr(separatorPos + 1);
    }

    for (const auto &item : m_screens[m_currentScreenId].items)
    {
        if (item.id == mainItemId)
        {
            ESP_LOGI(TAG, "-> Handling press for '%s'", item.label.c_str());

            if (item.type == "submenu")
            {
                if (!item.targetScreenId.empty() && m_screens.find(item.targetScreenId) != m_screens.end())
                {
                    m_currentScreenId = item.targetScreenId;
                    render(); // Render the new submenu!
                }
                else
                {
                    ESP_LOGW(TAG, "Target screen '%s' not found!", item.targetScreenId.c_str());
                }
            }
            else if (item.type == "action")
            {
                if (!item.actionTopic.empty())
                {
                    ESP_LOGI(TAG, "Calling ActionManager: %s", item.actionTopic.c_str());
                    const char *value = "true";
                    action_manager_execute(item.actionTopic.c_str(), value);
                    if (m_actionCallback)
                    {
                        m_actionCallback(item.id, item.actionTopic, value);
                    }
                }
            }
            else if (item.type == "toggle")
            {
                // Fetch the *actual* current state to determine what the *new* state should be
                bool currentState = item.toggleValue;
                if (m_valueProvider)
                {
                    std::string val = m_valueProvider(item.id);
                    if (!val.empty())
                        currentState = (val == "true");
                }

                bool newState = !currentState;
                const char *valStr = newState ? "true" : "false";
                ESP_LOGI(TAG, "Toggle item '%s' requested flip to %s", item.label.c_str(), valStr);

                if (!item.actionTopic.empty())
                {
                    action_manager_execute(item.actionTopic.c_str(), valStr);
                    if (m_actionCallback)
                    {
                        m_actionCallback(item.id, item.actionTopic, valStr);
                    }
                }
            }
            else if (item.type == "selection")
            {
                if (subValue.empty())
                {
                    ESP_LOGI(TAG, "Selection item '%s' pressed, opening options.", item.label.c_str());
                    // In a real UI, this would open the list of options.
                    // For our pseudo-UI, we do nothing and wait for a press with a sub-value.
                    return;
                }

                // A choice was made, validate it
                bool isValidChoice = false;
                for (const auto &sel_item : item.selectionItems)
                {
                    if (sel_item.value == subValue)
                    {
                        isValidChoice = true;
                        break;
                    }
                }

                if (isValidChoice)
                {
                    ESP_LOGI(TAG, "-> Chose option with value '%s' for '%s'", subValue.c_str(), item.label.c_str());
                    if (!item.actionTopic.empty())
                    {
                        action_manager_execute(item.actionTopic.c_str(), subValue.c_str());
                        if (m_actionCallback)
                        {
                            m_actionCallback(item.id, item.actionTopic, subValue);
                        }
                    }
                }
            }
            return; // Item found and processed
        }
    }

    ESP_LOGW(TAG, "Item ID '%s' not found in current screen", mainItemId.c_str());
}