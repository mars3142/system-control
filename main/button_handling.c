#include "button_handling.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>

#include "common.h"

static const char *TAG = "button_handling";

#define DEBOUNCE_TIME_MS (50) // Shorter debounce time for timer-based debouncing
#define BUTTON_QUEUE_LENGTH 5
#define BUTTON_QUEUE_ITEM_SIZE sizeof(uint8_t)

const uint8_t pins[] = {BUTTON_DOWN, BUTTON_UP, BUTTON_LEFT, BUTTON_RIGHT, BUTTON_SELECT, BUTTON_BACK};

QueueHandle_t buttonQueue = NULL;

// Structure for button state
typedef struct
{
    uint8_t pin;
    esp_timer_handle_t timer;
    bool is_pressed;
    int64_t last_interrupt_time;
} button_state_t;

// Array for button states
static button_state_t button_states[6];

// Timer callback for debouncing
static void button_timer_callback(void *arg)
{
    button_state_t *button = (button_state_t *)arg;

    // Check current GPIO state
    int level = gpio_get_level(button->pin);

    // Button is pressed (LOW) and was not pressed before
    if (level == 0 && !button->is_pressed)
    {
        button->is_pressed = true;

        // Send button press to queue
        uint8_t press_signal = button->pin;
        xQueueSend(buttonQueue, &press_signal, 0);

        ESP_LOGD(TAG, "Button %d pressed", button->pin);
    }
    // Button is released (HIGH) and was pressed before
    else if (level == 1 && button->is_pressed)
    {
        button->is_pressed = false;
        ESP_LOGD(TAG, "Button %d released", button->pin);
    }
}

// ISR Handler - only starts the timer
void IRAM_ATTR button_isr_handler(void *arg)
{
    button_state_t *button = (button_state_t *)arg;
    int64_t now = esp_timer_get_time();

    // Simple time-based debouncing in ISR
    if ((now - button->last_interrupt_time) > (DEBOUNCE_TIME_MS * 1000))
    {
        button->last_interrupt_time = now;

        // Start/restart the timer
        esp_timer_stop(button->timer);
        esp_timer_start_once(button->timer, DEBOUNCE_TIME_MS * 1000);
    }
}

void setup_buttons(void)
{
    buttonQueue = xQueueCreate(BUTTON_QUEUE_LENGTH, BUTTON_QUEUE_ITEM_SIZE);
    if (buttonQueue == NULL)
    {
        ESP_LOGE(TAG, "Error while Queue creation!");
        return;
    }
    ESP_LOGI(TAG, "Button Queue created.");

    esp_err_t isr_service_err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    if (isr_service_err != ESP_OK && isr_service_err != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "Error in gpio_install_isr_service: %s", esp_err_to_name(isr_service_err));
    }

    // Timer configuration
    esp_timer_create_args_t timer_args = {.callback = button_timer_callback, .name = "button_debounce"};

    for (int i = 0; i < sizeof(pins) / sizeof(pins[0]); i++)
    {
        const uint8_t pin = pins[i];

        // Initialize button state
        button_states[i].pin = pin;
        button_states[i].is_pressed = false;
        button_states[i].last_interrupt_time = 0;

        // Create timer for this button
        timer_args.arg = &button_states[i];
        esp_err_t timer_err = esp_timer_create(&timer_args, &button_states[i].timer);
        if (timer_err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to create timer for button %d: %s", pin, esp_err_to_name(timer_err));
            continue;
        }

        // GPIO configuration
        gpio_config_t io_conf = {.intr_type = GPIO_INTR_ANYEDGE, // React to both edges
                                 .pin_bit_mask = (1ULL << pin),
                                 .mode = GPIO_MODE_INPUT,
                                 .pull_up_en = GPIO_PULLUP_ENABLE,
                                 .pull_down_en = GPIO_PULLDOWN_DISABLE};
        gpio_config(&io_conf);

        // Add ISR handler
        esp_err_t add_isr_err = gpio_isr_handler_add(pin, button_isr_handler, &button_states[i]);
        if (add_isr_err != ESP_OK)
        {
            ESP_LOGE(TAG, "Error in gpio_isr_handler_add: %s", esp_err_to_name(add_isr_err));
        }

        ESP_LOGD(TAG, "Button interrupt configured for GPIO %d", pin);
    }
}

// Cleanup function (optional)
void cleanup_buttons(void)
{
    for (int i = 0; i < sizeof(pins) / sizeof(pins[0]); i++)
    {
        if (button_states[i].timer != NULL)
        {
            esp_timer_stop(button_states[i].timer);
            esp_timer_delete(button_states[i].timer);
        }
        gpio_isr_handler_remove(button_states[i].pin);
    }

    if (buttonQueue != NULL)
    {
        vQueueDelete(buttonQueue);
    }
}
