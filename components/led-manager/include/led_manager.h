#pragma once

#include "stdint.h"

enum
{
    EVENT_LED_ON,
    EVENT_LED_OFF,
    EVENT_LED_DAY,
    EVENT_LED_NIGHT,
};

typedef struct
{
    int value;
} led_event_data_t;

uint64_t wled_init(void);

uint64_t register_handler(void);

uint64_t send_event(uint32_t event, led_event_data_t *payload);
