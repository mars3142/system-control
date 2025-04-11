#include "button_handling.h"

#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "common.h"

static const char* TAG = "button_handling";

#define DEBOUNCE_TIME_MS (500)

#define BUTTON_QUEUE_LENGTH    5
#define BUTTON_QUEUE_ITEM_SIZE sizeof(uint8_t)

#define WLED_GPIO           GPIO_NUM_47
#define WLED_RMT_CHANNEL    RMT_CHANNEL_0
#define WLED_RESOLUTION_HZ  (10000000)
#define WLED_ON_DURATION_MS (100)
#define NUM_LEDS            (1)

const uint8_t pins[] =
    {BUTTON_DOWN, BUTTON_UP, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_SELECT, BUTTON_BACK};

QueueHandle_t buttonQueue = NULL;
volatile int64_t last_interrupt_time = 0;

void IRAM_ATTR button_isr_handler(void* arg) {
    int64_t now = esp_timer_get_time();
    if((now - last_interrupt_time) > (DEBOUNCE_TIME_MS * 1000)) {
        last_interrupt_time = now;

        uintptr_t pin_value = (uintptr_t)arg;
        uint8_t press_signal = (uint8_t)pin_value;
        BaseType_t higherPriorityTaskWoken = pdFALSE;

        xQueueSendFromISR(buttonQueue, &press_signal, &higherPriorityTaskWoken);

        if(higherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void setupButtons(void) {
    buttonQueue = xQueueCreate(BUTTON_QUEUE_LENGTH, BUTTON_QUEUE_ITEM_SIZE);
    if(buttonQueue == NULL) {
        ESP_LOGE(TAG, "Error while Queue creation!");
        return;
    }
    ESP_LOGI(TAG, "Button Queue created.");

    esp_err_t isr_service_err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    if(isr_service_err != ESP_OK && isr_service_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Error in gpio_install_isr_service: %s", esp_err_to_name(isr_service_err));
    }

    for(int i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
        const uint8_t pin = pins[i];
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_NEGEDGE;
        io_conf.pin_bit_mask = (1ULL << pin);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_config(&io_conf);

        uintptr_t pin_as_arg = (uintptr_t)pin;
        esp_err_t add_isr_err = gpio_isr_handler_add(pin, button_isr_handler, (void*)pin_as_arg);
        if(add_isr_err != ESP_OK) {
            ESP_LOGE(TAG, "Error in gpio_isr_handler_add: %s", esp_err_to_name(add_isr_err));
        }

        ESP_LOGI(TAG, "Button interrupt configured for GPIO %d", pin);
    }
}
