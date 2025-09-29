#pragma once

#include <stdint.h>

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_t;

void interpolate_color(const rgb_t start_color, const rgb_t end_color, float fraction, rgb_t *out_color);
