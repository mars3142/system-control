#pragma once

#include <stddef.h>
#include <stdint.h>

#define LED_SEGMENT_MAX_LEN 15

typedef struct
{
    char name[32];
    uint16_t start;
    uint16_t leds;
} led_segment_t;

led_segment_t segments[LED_SEGMENT_MAX_LEN];
size_t segment_count;
