#pragma once

#include "color.h"
#include <esp_check.h>
#include <stdint.h>

// Number of LEDs to be controlled
#define STATUS_LED_COUNT 3

// Possible lighting modes
typedef enum
{
    LED_MODE_OFF,
    LED_MODE_SOLID,
    LED_MODE_BLINK,
    LED_MODE_BLINK_ALT // alternates between color and alt_color
} led_mode_t;

// This is the structure you pass from the outside to define a behavior
typedef struct
{
    uint32_t on_time_ms;  // Only relevant for BLINK / BLINK_ALT
    uint32_t off_time_ms; // Only relevant for BLINK / BLINK_ALT
    rgb_t color;
    rgb_t alt_color; // Only relevant for BLINK_ALT
    uint8_t index;
    led_mode_t mode;
} led_behavior_t;

__BEGIN_DECLS
/**
 * @brief Initializes the status manager and the LED strip.
 *
 * @param gpio_num GPIO where the data line of the LEDs is connected.
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t led_status_init(int gpio_num);

/**
 * @brief Sets the lighting behavior for a single LED.
 *
 * This function is thread-safe.
 *
 * @param index Index of the LED (0 to STATUS_LED_COUNT - 1).
 * @param behavior The structure with the desired behavior.
 * @return esp_err_t ESP_OK on success, ESP_ERR_INVALID_ARG on invalid index.
 */
esp_err_t led_status_set_behavior(led_behavior_t behavior);
__END_DECLS
