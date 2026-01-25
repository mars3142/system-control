#pragma once

#include <stdint.h>
#include <sys/cdefs.h>

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb_t;

typedef struct
{
    float h;
    float s;
    float v;
} hsv_t;

__BEGIN_DECLS
rgb_t interpolate_color_rgb(rgb_t start, rgb_t end, float factor);
rgb_t interpolate_color_hsv(rgb_t start, rgb_t end, float factor);
hsv_t rgb_to_hsv(rgb_t rgb);
rgb_t hsv_to_rgb(hsv_t hsv);
__END_DECLS
