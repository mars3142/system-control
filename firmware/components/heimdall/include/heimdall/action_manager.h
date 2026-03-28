#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Signatur für Callback-Funktionen (reines C)
     * @param value Der übergebene Wert als C-String (z.B. "true", "false", "2")
     */
    typedef void (*action_callback_t)(const char *value);

    /**
     * @brief Registriert eine Aktion, die über die dynamische UI aufgerufen werden kann
     */
    void action_manager_register(const char *action_name, action_callback_t callback);

    /**
     * @brief Führt eine registrierte Aktion aus
     */
    void action_manager_execute(const char *action_name, const char *value);

#ifdef __cplusplus
}
#endif
