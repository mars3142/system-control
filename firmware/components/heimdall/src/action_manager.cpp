#include "heimdall/action_manager.h"
#include <esp_log.h>
#include <string>
#include <unordered_map>

static const char *TAG = "ActionMgr";

// Hier speichern wir alle registrierten C-Funktionszeiger
static std::unordered_map<std::string, action_callback_t> s_actions;

extern "C" void action_manager_register(const char *action_name, action_callback_t callback)
{
    if (action_name && callback)
    {
        s_actions[action_name] = callback;
        ESP_LOGD(TAG, "Aktion registriert: %s", action_name);
    }
}

extern "C" void action_manager_execute(const char *action_name, const char *value)
{
    if (!action_name)
        return;

    auto it = s_actions.find(action_name);
    if (it != s_actions.end())
    {
        ESP_LOGI(TAG, "Führe Aktion aus: %s (Wert: %s)", action_name, value ? value : "NULL");
        // Ruft den hinterlegten C-Funktionszeiger auf
        it->second(value ? value : "");
    }
    else
    {
        ESP_LOGW(TAG, "Aktion nicht gefunden: %s", action_name);
    }
}
