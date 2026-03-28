#include "mercedes/mercedes.h"
#include "heimdall/action_manager.h"
#include "message_manager.h"
#include "persistence_manager.h"

#include <cJSON.h>
#include <cstdlib>
#include <cstring>
#include <esp_log.h>

static persistence_manager_t s_pm;
static bool s_pm_initialized = false;

static void ensure_pm()
{
    if (!s_pm_initialized)
    {
        s_pm_initialized = (persistence_manager_init(&s_pm, "config") == ESP_OK);
    }
}

static void post_settings_message(const MenuItemDef &item, const std::string &value)
{
    if (!item.persistent)
        return;

    message_t msg = {};
    msg.type = MESSAGE_TYPE_SETTINGS;
    strncpy(msg.data.settings.key, item.id.c_str(), sizeof(msg.data.settings.key) - 1);

    if (item.valueType == "int")
    {
        msg.data.settings.type = SETTINGS_TYPE_INT;
        msg.data.settings.value.int_value = atoi(value.c_str());
    }
    else if (item.valueType == "bool" || item.type == "toggle")
    {
        msg.data.settings.type = SETTINGS_TYPE_BOOL;
        msg.data.settings.value.bool_value = (value == "true");
    }
    else
    {
        msg.data.settings.type = SETTINGS_TYPE_STRING;
        strncpy(msg.data.settings.value.string_value, value.c_str(),
                sizeof(msg.data.settings.value.string_value) - 1);
    }
    message_manager_post(&msg, pdMS_TO_TICKS(10));
}

static bool nvs_read_item(const MenuItemDef &item, char *buf, size_t bufSize)
{
    if (!item.persistent)
        return false;
    ensure_pm();
    if (!s_pm_initialized)
        return false;

    if (item.valueType == "int")
    {
        int32_t v = 0;
        if (nvs_get_i32(s_pm.nvs_handle, item.id.c_str(), &v) != ESP_OK)
            return false;
        snprintf(buf, bufSize, "%d", (int)v);
        return true;
    }
    else if (item.valueType == "bool" || item.type == "toggle")
    {
        uint8_t v = 0;
        if (nvs_get_u8(s_pm.nvs_handle, item.id.c_str(), &v) != ESP_OK)
            return false;
        snprintf(buf, bufSize, "%s", v ? "true" : "false");
        return true;
    }
    else
    {
        if (!persistence_manager_has_key(&s_pm, item.id.c_str()))
            return false;
        persistence_manager_get_string(&s_pm, item.id.c_str(), buf, bufSize, "");
        return buf[0] != '\0';
    }
}

static const char *TAG = "Mercedes";

Mercedes &Mercedes::getInstance()
{
    static Mercedes instance;
    return instance;
}

Mercedes::Mercedes()
{
}

void Mercedes::setActionCallback(MenuActionCallback callback)
{
    m_actionCallback = callback;
}

void Mercedes::setItemValueProvider(ItemValueProvider provider)
{
    m_valueProvider = provider;
}

void Mercedes::setStateChangedCallback(MenuStateChangedCallback callback)
{
    m_stateChangedCallback = callback;
}

bool Mercedes::buildFromJson(const std::string &jsonPayload)
{
    cJSON *root = cJSON_Parse(jsonPayload.c_str());
    if (!root)
    {
        ESP_LOGE(TAG, "Error parsing JSON payload");
        return false;
    }

    m_screens.clear();
    m_currentScreenId = "";
    m_selectedIndex = 0;
    while (!m_screenHistory.empty())
        m_screenHistory.pop();

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

                    cJSON *valueTypeItem = cJSON_GetObjectItem(item, "valueType");
                    if (valueTypeItem && cJSON_IsString(valueTypeItem))
                        itemDef.valueType = valueTypeItem->valuestring;

                    cJSON *valueItem = cJSON_GetObjectItem(item, "value");
                    if (valueItem && cJSON_IsBool(valueItem))
                        itemDef.toggleValue = cJSON_IsTrue(valueItem);

                    cJSON *visibleWhen = cJSON_GetObjectItem(item, "visibleWhen");
                    if (visibleWhen)
                    {
                        cJSON *whenId = cJSON_GetObjectItem(visibleWhen, "itemId");
                        cJSON *whenVal = cJSON_GetObjectItem(visibleWhen, "value");
                        if (whenId && cJSON_IsString(whenId))
                            itemDef.visibleWhenItemId = whenId->valuestring;
                        if (whenVal && cJSON_IsString(whenVal))
                            itemDef.visibleWhenValue = whenVal->valuestring;
                    }

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

    cJSON_Delete(root);

    // Restore persistent item states from NVS
    for (auto &[screenId, screen] : m_screens)
    {
        for (auto &item : screen.items)
        {
            if (!item.persistent)
                continue;
            char val[32] = {};
            if (!nvs_read_item(item, val, sizeof(val)))
                continue;

            if (item.type == "toggle")
            {
                item.toggleValue = (strcmp(val, "true") == 0);
            }
            else if (item.type == "selection")
            {
                for (int i = 0; i < static_cast<int>(item.selectionItems.size()); i++)
                {
                    if (item.selectionItems[i].value == val)
                    {
                        item.selectionIndex = i;
                        break;
                    }
                }
            }
        }
    }

    if (m_stateChangedCallback)
        m_stateChangedCallback();

    return true;
}

// --- Navigation ---

void Mercedes::handleInput(button_type_t button)
{
    const MenuScreenDef *screen = getCurrentScreen();
    if (!screen || screen->items.empty())
        return;

    switch (button)
    {
    case BTN_UP:
    {
        int idx = static_cast<int>(m_selectedIndex);
        int count = static_cast<int>(screen->items.size());
        do {
            idx = (idx == 0) ? count - 1 : idx - 1;
        } while (!isItemVisible(screen->items[idx]) && idx != static_cast<int>(m_selectedIndex));
        m_selectedIndex = static_cast<size_t>(idx);
        if (m_stateChangedCallback)
            m_stateChangedCallback();
        break;
    }

    case BTN_DOWN:
    {
        int idx = static_cast<int>(m_selectedIndex);
        int count = static_cast<int>(screen->items.size());
        do {
            idx = (idx + 1) % count;
        } while (!isItemVisible(screen->items[idx]) && idx != static_cast<int>(m_selectedIndex));
        m_selectedIndex = static_cast<size_t>(idx);
        if (m_stateChangedCallback)
            m_stateChangedCallback();
        break;
    }

    case BTN_LEFT:
    case BTN_RIGHT:
        adjustCurrentItem(button);
        break;

    case BTN_SELECT:
        activateCurrentItem();
        break;

    case BTN_BACK:
        if (!m_screenHistory.empty())
        {
            m_currentScreenId = m_screenHistory.top().first;
            m_selectedIndex = m_screenHistory.top().second;
            m_screenHistory.pop();
            if (m_stateChangedCallback)
                m_stateChangedCallback();
        }
        break;

    default:
        break;
    }
}

void Mercedes::navigateToScreen(const std::string &screenId)
{
    if (m_screens.find(screenId) == m_screens.end())
    {
        ESP_LOGW(TAG, "Target screen '%s' not found", screenId.c_str());
        return;
    }

    m_screenHistory.push({m_currentScreenId, m_selectedIndex});
    m_currentScreenId = screenId;
    m_selectedIndex = 0;

    const MenuScreenDef &newScreen = m_screens[screenId];
    for (size_t i = 0; i < newScreen.items.size(); i++)
    {
        if (isItemVisible(newScreen.items[i]))
        {
            m_selectedIndex = i;
            break;
        }
    }

    if (m_stateChangedCallback)
        m_stateChangedCallback();
}

void Mercedes::activateCurrentItem()
{
    const MenuScreenDef *screen = getCurrentScreen();
    if (!screen || m_selectedIndex >= screen->items.size())
        return;

    MenuItemDef &item = m_screens[m_currentScreenId].items[m_selectedIndex];

    if (item.type == "label")
        return;

    if (item.type == "submenu")
    {
        if (!item.targetScreenId.empty())
            navigateToScreen(item.targetScreenId);
    }
    else if (item.type == "action")
    {
        if (!item.actionTopic.empty())
        {
            action_manager_execute(item.actionTopic.c_str(), "true");
            if (m_actionCallback)
                m_actionCallback(item.id, item.actionTopic, "true");
        }
    }
    else if (item.type == "toggle")
    {
        bool currentState = item.toggleValue;
        if (m_valueProvider)
        {
            char val[32] = {};
            m_valueProvider(item.id, val, sizeof(val));
            if (val[0] != '\0')
                currentState = (strcmp(val, "true") == 0);
        }

        bool newState = !currentState;
        item.toggleValue = newState;
        const char *valStr = newState ? "true" : "false";

        post_settings_message(item, valStr);

        if (!item.actionTopic.empty())
        {
            action_manager_execute(item.actionTopic.c_str(), valStr);
            if (m_actionCallback)
                m_actionCallback(item.id, item.actionTopic, valStr);
        }

        if (m_stateChangedCallback)
            m_stateChangedCallback();
    }
}

void Mercedes::adjustCurrentItem(button_type_t button)
{
    const MenuScreenDef *screen = getCurrentScreen();
    if (!screen || m_selectedIndex >= screen->items.size())
        return;

    MenuItemDef &item = m_screens[m_currentScreenId].items[m_selectedIndex];

    if (item.type == "selection" && !item.selectionItems.empty())
    {
        int count = static_cast<int>(item.selectionItems.size());
        if (button == BTN_LEFT)
            item.selectionIndex = (item.selectionIndex == 0) ? count - 1 : item.selectionIndex - 1;
        else if (button == BTN_RIGHT)
            item.selectionIndex = (item.selectionIndex + 1) % count;

        const std::string &selectedValue = item.selectionItems[item.selectionIndex].value;

        post_settings_message(item, selectedValue);

        if (!item.actionTopic.empty())
        {
            action_manager_execute(item.actionTopic.c_str(), selectedValue.c_str());
            if (m_actionCallback)
                m_actionCallback(item.id, item.actionTopic, selectedValue);
        }

        if (m_stateChangedCallback)
            m_stateChangedCallback();
    }
}

// --- State accessors ---

const MenuScreenDef *Mercedes::getCurrentScreen() const
{
    auto it = m_screens.find(m_currentScreenId);
    if (it == m_screens.end())
        return nullptr;
    return &it->second;
}

size_t Mercedes::getSelectedIndex() const
{
    return m_selectedIndex;
}

bool Mercedes::canGoBack() const
{
    return !m_screenHistory.empty();
}

const ItemValueProvider *Mercedes::getItemValueProvider() const
{
    return m_valueProvider ? &m_valueProvider : nullptr;
}

bool Mercedes::isItemVisible(const MenuItemDef &item) const
{
    if (item.visibleWhenItemId.empty())
        return true;

    for (const auto &[screenId, screen] : m_screens)
    {
        for (const auto &other : screen.items)
        {
            if (other.id != item.visibleWhenItemId)
                continue;

            if (m_valueProvider)
            {
                char val[32] = {};
                m_valueProvider(other.id, val, sizeof(val));
                if (val[0] != '\0')
                    return strcmp(val, item.visibleWhenValue.c_str()) == 0;
            }

            if (other.type == "selection" && !other.selectionItems.empty())
                return other.selectionItems[other.selectionIndex].value == item.visibleWhenValue;

            if (other.type == "toggle")
                return (item.visibleWhenValue == "true") == other.toggleValue;

            return false;
        }
    }
    return true;
}

void Mercedes::updateItemValue(const std::string &id, const std::string &value)
{
    for (auto &[screenId, screen] : m_screens)
    {
        for (auto &item : screen.items)
        {
            if (item.id != id)
                continue;

            if (item.type == "toggle")
            {
                item.toggleValue = (value == "true");
            }
            else if (item.type == "selection")
            {
                for (int i = 0; i < static_cast<int>(item.selectionItems.size()); i++)
                {
                    if (item.selectionItems[i].value == value)
                    {
                        item.selectionIndex = i;
                        break;
                    }
                }
            }

            if (m_stateChangedCallback)
                m_stateChangedCallback();
            return;
        }
    }
}
