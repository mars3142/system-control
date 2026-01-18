#include "message_manager.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <persistence_manager.h>
#include <string.h>

#define MESSAGE_QUEUE_LENGTH 16
#define MESSAGE_QUEUE_ITEM_SIZE sizeof(message_t)

static const char *TAG = "message_manager";
static QueueHandle_t message_queue = NULL;

// Observer Pattern: Listener-Liste
#define MAX_MESSAGE_LISTENERS 8
static message_listener_t message_listeners[MAX_MESSAGE_LISTENERS] = {0};
static size_t message_listener_count = 0;

void message_manager_register_listener(message_listener_t listener)
{
    if (listener && message_listener_count < MAX_MESSAGE_LISTENERS)
    {
        // Doppelte Registrierung vermeiden
        for (size_t i = 0; i < message_listener_count; ++i)
        {
            if (message_listeners[i] == listener)
                return;
        }
        message_listeners[message_listener_count++] = listener;
    }
}

void message_manager_unregister_listener(message_listener_t listener)
{
    for (size_t i = 0; i < message_listener_count; ++i)
    {
        if (message_listeners[i] == listener)
        {
            // Nachfolgende Listener nach vorne schieben
            for (size_t j = i; j < message_listener_count - 1; ++j)
            {
                message_listeners[j] = message_listeners[j + 1];
            }
            message_listeners[--message_listener_count] = NULL;
            break;
        }
    }
}

static void message_manager_task(void *param)
{
    message_t msg;
    persistence_manager_t pm;
    while (1)
    {
        if (xQueueReceive(message_queue, &msg, portMAX_DELAY) == pdTRUE)
        {
            switch (msg.type)
            {
            case MESSAGE_TYPE_SETTINGS:
                if (persistence_manager_init(&pm, "config") == ESP_OK)
                {
                    switch (msg.data.settings.type)
                    {
                    case SETTINGS_TYPE_BOOL:
                        persistence_manager_set_bool(&pm, msg.data.settings.key, msg.data.settings.value.bool_value);
                        break;
                    case SETTINGS_TYPE_INT:
                        persistence_manager_set_int(&pm, msg.data.settings.key, msg.data.settings.value.int_value);
                        break;
                    case SETTINGS_TYPE_FLOAT:
                        persistence_manager_set_float(&pm, msg.data.settings.key, msg.data.settings.value.float_value);
                        break;
                    case SETTINGS_TYPE_STRING:
                        persistence_manager_set_string(&pm, msg.data.settings.key,
                                                       msg.data.settings.value.string_value);
                        break;
                    }
                    persistence_manager_deinit(&pm);
                    ESP_LOGD(TAG, "Setting written: %s", msg.data.settings.key);
                }
                break;
            case MESSAGE_TYPE_BUTTON:
                ESP_LOGD(TAG, "Button event: id=%d, type=%d", msg.data.button.button_id, msg.data.button.event_type);
                break;
            case MESSAGE_TYPE_SIMULATION:
                /// just logging
                ESP_LOGD(TAG, "Simulation event: time=%s, color=(%d,%d,%d)", msg.data.simulation.time,
                         msg.data.simulation.red, msg.data.simulation.green, msg.data.simulation.blue);
                break;
            }
            // Observer Pattern: Listener benachrichtigen
            for (size_t i = 0; i < message_listener_count; ++i)
            {
                if (message_listeners[i])
                {
                    message_listeners[i](&msg);
                }
            }
        }
    }
}

void message_manager_init(void)
{
    if (!message_queue)
    {
        message_queue = xQueueCreate(MESSAGE_QUEUE_LENGTH, MESSAGE_QUEUE_ITEM_SIZE);
        xTaskCreate(message_manager_task, "message_manager_task", 4096, NULL, 5, NULL);
    }
}

bool message_manager_post(const message_t *msg, TickType_t timeout)
{
    if (!message_queue)
        return false;
    ESP_LOGD(TAG, "Post: type=%d", msg->type);
    return xQueueSend(message_queue, msg, timeout) == pdTRUE;
}
