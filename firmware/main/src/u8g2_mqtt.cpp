#include "u8g2_mqtt.h"

#include "esp_timer.h"
#include <my_mqtt_client.h>
#include <stdint.h>
#include <string.h>
#include <u8g2.h>

#define BUFFER_SIZE (128 * 64 / 8)
QueueHandle_t display_mqtt_queue = nullptr;

void u8g2_mqtt_task(void *pvParameters)
{
    static uint8_t current_buffer[BUFFER_SIZE];
    static uint8_t previous_buffer[BUFFER_SIZE] = {0};
    static uint8_t mqtt_payload[BUFFER_SIZE * 2 + 1];

    uint64_t last_keyframe_time = 0;
    const uint64_t KEYFRAME_INTERVAL_US = 5000000; // 5 seconds in microseconds

    while (true)
    {
        // Blocks without CPU load until app_task provides a frame
        if (xQueueReceive(display_mqtt_queue, current_buffer, portMAX_DELAY) == pdTRUE)
        {
            int payload_size = 0;
            uint64_t current_time = esp_timer_get_time();

            // Time-based I-frame decision (or on initial start)
            bool is_keyframe = (current_time - last_keyframe_time >= KEYFRAME_INTERVAL_US) || (last_keyframe_time == 0);

            if (is_keyframe)
            {
                mqtt_payload[payload_size++] = 0x01; // Header: I-frame
                last_keyframe_time = current_time;

                for (int i = 0; i < BUFFER_SIZE;)
                {
                    uint8_t count = 1;
                    while (i + count < BUFFER_SIZE && current_buffer[i] == current_buffer[i + count] && count < 255)
                    {
                        count++;
                    }
                    mqtt_payload[payload_size++] = count;
                    mqtt_payload[payload_size++] = current_buffer[i];
                    i += count;
                }
            }
            else
            {
                mqtt_payload[payload_size++] = 0x00; // Header: P-frame (Diff)
                uint8_t xor_buffer[BUFFER_SIZE];

                for (int i = 0; i < BUFFER_SIZE; i++)
                {
                    xor_buffer[i] = current_buffer[i] ^ previous_buffer[i];
                }

                for (int i = 0; i < BUFFER_SIZE;)
                {
                    uint8_t count = 1;
                    while (i + count < BUFFER_SIZE && xor_buffer[i] == xor_buffer[i + count] && count < 255)
                    {
                        count++;
                    }
                    mqtt_payload[payload_size++] = count;
                    mqtt_payload[payload_size++] = xor_buffer[i];
                    i += count;
                }
            }

            // --- MQTT SEND ---
            mqtt_client_publish("stream", (char *)mqtt_payload, payload_size, 0, false);

            memcpy(previous_buffer, current_buffer, BUFFER_SIZE);
        }
    }
}
