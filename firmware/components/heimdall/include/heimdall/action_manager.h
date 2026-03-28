#pragma once

#include "heimdall/button_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Callback signature for action handlers
     * @param value The value passed as a C string (e.g. "true", "false", "2")
     */
    typedef void (*action_callback_t)(const char *value);

    /**
     * @brief Registers an action that can be triggered via the dynamic UI or API
     */
    void action_manager_register(const char *action_name, action_callback_t callback);

    /**
     * @brief Executes a registered action by name
     */
    void action_manager_execute(const char *action_name, const char *value);

#ifdef __cplusplus
}
#endif
