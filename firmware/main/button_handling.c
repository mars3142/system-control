#include "button_handling.h"

#include "button_gpio.h"
#include "common.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_insights.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "iot_button.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "button_handling";

const uint8_t gpios[] = {BUTTON_DOWN, BUTTON_UP, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_SELECT, BUTTON_BACK};

typedef struct
{
    uint8_t gpio;
} button_user_data_t;

static button_user_data_t button_data[6];

QueueHandle_t buttonQueue = NULL;

static void button_event_cb(void *arg, void *usr_data)
{
    if (buttonQueue == NULL)
    {
        ESP_LOGE(TAG, "Button queue not initialized!");
        return;
    }
    button_user_data_t *data = (button_user_data_t *)usr_data;
    uint8_t gpio_num = data->gpio;

    ESP_LOGI(TAG, "Button pressed on GPIO %d", gpio_num);

    if (xQueueSend(buttonQueue, &gpio_num, 0) != pdTRUE)
    {
        ESP_LOGW(TAG, "Failed to send button press to queue");
    }
}

static void init_button(uint8_t gpio, int index)
{
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = gpio,
        .active_level = 0,
        .enable_power_save = true,
    };
    button_handle_t gpio_btn = NULL;
    const esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &gpio_btn);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Button create failed");
    }

    button_data[index].gpio = gpio;
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, NULL, button_event_cb, &button_data[index]);
}

void setup_buttons(void)
{
    buttonQueue = xQueueCreate(10, sizeof(uint8_t));
    if (buttonQueue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create button queue");
        return;
    }

    ESP_DIAG_EVENT(TAG, "Button queue created successfully");
    for (int i = 0; i < sizeof(gpios) / sizeof(gpios[0]); i++)
    {
        init_button(gpios[i], i);
    }
}

// Cleanup function (optional)
void cleanup_buttons(void)
{
    if (buttonQueue != NULL)
    {
        vQueueDelete(buttonQueue);
    }
}
