#include "led_status.h"

#include "esp_log.h"
#include "esp_timer.h" // For high-resolution timestamps
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "led_strip.h"

static const char *TAG = "led_status";

// Internal control structure for each LED
typedef struct
{
    led_behavior_t behavior;      // The desired behavior (target state)
    uint64_t last_toggle_time_us; // Timestamp of the last toggle (in microseconds)
    bool is_on_in_blink;          // Current state in blink mode (actual state)
} led_control_t;

// --- Module variables ---
static led_strip_handle_t led_strip;
static led_control_t led_controls[STATUS_LED_COUNT];
static SemaphoreHandle_t mutex; // To protect the led_controls array

// The core: The task that controls the LEDs
static void led_status_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Led Status Task started.");

    while (true)
    {
        uint64_t now_us = esp_timer_get_time();

        // Take the mutex to safely access the control data
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
        {

            for (int i = 0; i < STATUS_LED_COUNT; i++)
            {
                led_control_t *control = &led_controls[i];

                switch (control->behavior.mode)
                {
                case LED_MODE_OFF:
                    led_strip_set_pixel(led_strip, i, 0, 0, 0);
                    break;

                case LED_MODE_SOLID:
                    led_strip_set_pixel(led_strip, i, control->behavior.color.r, control->behavior.color.g,
                                        control->behavior.color.b);
                    break;

                case LED_MODE_BLINK: {
                    uint32_t duration_ms =
                        control->is_on_in_blink ? control->behavior.on_time_ms : control->behavior.off_time_ms;
                    if ((now_us - control->last_toggle_time_us) / 1000 >= duration_ms)
                    {
                        control->is_on_in_blink = !control->is_on_in_blink; // Toggle state
                        control->last_toggle_time_us = now_us;              // Update timestamp
                    }

                    if (control->is_on_in_blink)
                    {
                        led_strip_set_pixel(led_strip, i, control->behavior.color.r, control->behavior.color.g,
                                            control->behavior.color.b);
                    }
                    else
                    {
                        led_strip_set_pixel(led_strip, i, 0, 0, 0);
                    }
                }
                break;
                }
            }
            // Release the mutex
            xSemaphoreGive(mutex);
        }

        // Update the physical LED strip with the new values
        led_strip_refresh(led_strip);

        // Delay task for 20ms before the next iteration
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// Initialization function
esp_err_t led_status_init(int gpio_num)
{
    // LED strip configuration (e.g., for WS2812)
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_num,
        .max_leds = STATUS_LED_COUNT,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRBW,
        .flags =
            {
                .invert_out = false,
            },
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 0,
        .flags =
            {
                .with_dma = false,
            },
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "LED strip initialized.");

    // Create mutex
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL)
    {
        ESP_LOGE(TAG, "Could not create mutex.");
        return ESP_FAIL;
    }

    // Start task
    xTaskCreate(led_status_task, "led_status_task", 2048, NULL, 5, NULL);

    return ESP_OK;
}

// Function to set the behavior
esp_err_t led_status_set_behavior(uint8_t index, led_behavior_t behavior)
{
    if (index >= STATUS_LED_COUNT)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)
    {
        led_controls[index].behavior = behavior;
        // Reset internal state variables to start the new pattern cleanly
        led_controls[index].is_on_in_blink = false;
        led_controls[index].last_toggle_time_us = esp_timer_get_time();
        xSemaphoreGive(mutex);
    }

    return ESP_OK;
}
