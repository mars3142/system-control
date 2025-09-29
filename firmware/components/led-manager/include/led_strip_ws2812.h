#pragma once

#include "color.h"
#include <esp_check.h>
#include <sys/cdefs.h>

typedef enum
{
    LED_STATE_OFF,
    LED_STATE_DAY,
    LED_STATE_NIGHT,
    LED_STATE_SIMULATION,
} led_state_t;

__BEGIN_DECLS
esp_err_t led_strip_init(void);
esp_err_t led_strip_update(led_state_t state, rgb_t color);
__END_DECLS
