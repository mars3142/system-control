#pragma once

#include <freertos/FreeRTOS.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        MESSAGE_TYPE_SETTINGS,
        MESSAGE_TYPE_BUTTON
    } message_type_t;

    typedef enum
    {
        BUTTON_EVENT_PRESS,
        BUTTON_EVENT_RELEASE
    } button_event_type_t;

    typedef struct
    {
        button_event_type_t event_type;
        uint8_t button_id;
    } button_message_t;

    typedef enum
    {
        SETTINGS_TYPE_BOOL,
        SETTINGS_TYPE_INT,
        SETTINGS_TYPE_FLOAT,
        SETTINGS_TYPE_STRING
    } settings_type_t;

    typedef struct
    {
        settings_type_t type;
        char key[32];
        union {
            bool bool_value;
            int32_t int_value;
            float float_value;
            char string_value[64];
        } value;
    } settings_message_t;

    typedef struct
    {
        message_type_t type;
        union {
            settings_message_t settings;
            button_message_t button;
        } data;
    } message_t;

    // Observer Pattern: Listener-Typ und Registrierungsfunktionen
    typedef void (*message_listener_t)(const message_t *msg);
    void message_manager_register_listener(message_listener_t listener);
    void message_manager_unregister_listener(message_listener_t listener);
    void message_manager_init(void);
    bool message_manager_post(const message_t *msg, TickType_t timeout);

#ifdef __cplusplus
}
#endif
