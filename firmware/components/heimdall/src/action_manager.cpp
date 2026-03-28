#include "heimdall/action_manager.h"

#include <esp_log.h>
#include <string>
#include <unordered_map>

static const char *TAG = "ActionMgr";

static std::unordered_map<std::string, action_callback_t> s_actions;

extern "C" void action_manager_register(const char *action_name, action_callback_t callback)
{
    if (action_name && callback)
    {
        s_actions[action_name] = callback;
        ESP_LOGD(TAG, "Action registered: %s", action_name);
    }
}

extern "C" void action_manager_execute(const char *action_name, const char *value)
{
    if (!action_name)
        return;

    auto it = s_actions.find(action_name);
    if (it != s_actions.end())
    {
        ESP_LOGI(TAG, "Executing action: %s (value: %s)", action_name, value ? value : "NULL");
        it->second(value ? value : "");
    }
    else
    {
        ESP_LOGW(TAG, "Action not found: %s", action_name);
    }
}
