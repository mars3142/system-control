#include "led_manager.h"

#include "esp_event.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

led_strip_handle_t led_strip;

static const uint32_t value = 5;

ESP_EVENT_DECLARE_BASE(LED_EVENTS_BASE);
ESP_EVENT_DEFINE_BASE(LED_EVENTS_BASE);

esp_event_loop_handle_t loop_handle;

const char *TAG = "LED";

uint64_t wled_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_WLED_DIN_PIN,
        .max_leds = 500,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags =
            {
                .invert_out = false,
            },
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 0,
        .mem_block_symbols = 0,
        .flags =
            {
                .with_dma = true,
            },
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    for (uint32_t i = 0; i < 3; i++)
    {
        led_strip_set_pixel(led_strip, i, 10, 10, 0);
    }
    led_strip_refresh(led_strip);

    return ESP_OK;
}

void event_handler(void *arg, esp_event_base_t base, int32_t id, void *event_data)
{
    if (id == EVENT_LED_ON || id == EVENT_LED_OFF)
    {
        auto brightness = (id == EVENT_LED_ON) ? value : 0;
        for (uint32_t i = 0; i < 500; i++)
        {
            led_strip_set_pixel(led_strip, i, brightness, brightness, brightness);
        }
        led_strip_refresh(led_strip);
    }
}

uint64_t register_handler(void)
{
    esp_event_loop_args_t loop_args = {
        .queue_size = 2, .task_name = "led_manager", .task_priority = 5, .task_stack_size = 4096, .task_core_id = 1};
    esp_event_loop_create(&loop_args, &loop_handle);

    esp_event_handler_register_with(loop_handle, LED_EVENTS_BASE, ESP_EVENT_ANY_ID, event_handler, NULL);
    return ESP_OK;
}

uint64_t send_event(uint32_t event, led_event_data_t *payload)
{
    if (payload == nullptr)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = esp_event_post_to(loop_handle,              // Event loop handle
                                      LED_EVENTS_BASE,          // Event base
                                      event,                    // Event ID (EVENT_LED_ON, EVENT_LED_OFF, etc.)
                                      payload,                  // Data pointer
                                      sizeof(led_event_data_t), // Data size
                                      portMAX_DELAY             // Wait time
    );

    if (err != ESP_OK)
    {
        ESP_LOGE("LED", "Failed to post event: %s", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}
