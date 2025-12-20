#include "led_strip_ws2812.h"
#include "color.h"
#include "led_status.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <led_strip.h>
#include <sdkconfig.h>

static const char *TAG = "led_strip";

static led_strip_handle_t led_strip;
static QueueHandle_t led_command_queue;

static const uint32_t MAX_LEDS = CONFIG_LED_STRIP_MAX_LEDS;

typedef struct
{
    led_state_t state;
    rgb_t color;
} led_command_t;

static void set_all_pixels(const rgb_t color)
{
    for (uint32_t i = 0; i < MAX_LEDS; i++)
    {
        led_strip_set_pixel(led_strip, i, color.red, color.green, color.blue);
    }
    led_strip_refresh(led_strip);

    led_behavior_t led_behavior = {
        .index = 2,
        .mode = LED_MODE_SOLID,
        .color = {.red = color.red, .green = color.green, .blue = color.blue},
    };
    led_status_set_behavior(led_behavior);
}

void led_strip_task(void *pvParameters)
{
    led_state_t current_state = LED_STATE_OFF;
    led_command_t cmd;

    for (;;)
    {
        TickType_t wait_ticks = (current_state == LED_STATE_SIMULATION) ? pdMS_TO_TICKS(50) : portMAX_DELAY;
        if (xQueueReceive(led_command_queue, &cmd, wait_ticks) == pdPASS)
        {
            current_state = cmd.state;
        }

        rgb_t color;
        switch (current_state)
        {
        case LED_STATE_OFF:
            color = (rgb_t){.red = 0, .green = 0, .blue = 0};
            break;
        default:
            color = cmd.color;
            break;
        }

        set_all_pixels(color);
    }
};

esp_err_t led_strip_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_WLED_DIN_PIN,
        .max_leds = MAX_LEDS,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {.invert_out = false},
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 0,
        .mem_block_symbols = 0,
        .flags = {.with_dma = true},
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    led_command_queue = xQueueCreate(5, sizeof(led_command_t));
    if (led_command_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create command queue");
        return ESP_FAIL;
    }

    set_all_pixels((rgb_t){.red = 0, .green = 0, .blue = 0});

    xTaskCreatePinnedToCore(led_strip_task, "led_strip_task", 4096, NULL, tskIDLE_PRIORITY + 1, NULL,
                            CONFIG_FREERTOS_NUMBER_OF_CORES - 1);

    ESP_LOGI(TAG, "LED strip initialized");

    return ESP_OK;
}

esp_err_t led_strip_update(led_state_t state, rgb_t color)
{
    led_command_t cmd = {
        .state = state,
        .color = color,
    };

    if (xQueueSend(led_command_queue, &cmd, pdMS_TO_TICKS(100)) != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to send command to LED manager queue");
        return ESP_FAIL;
    }
    return ESP_OK;
}
