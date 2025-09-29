#include "color.h"

void interpolate_color(const rgb_t start_color, const rgb_t end_color, float fraction, rgb_t *out_color)
{
    out_color->r = start_color.r + (end_color.r - start_color.r) * fraction;
    out_color->g = start_color.g + (end_color.g - start_color.g) * fraction;
    out_color->b = start_color.b + (end_color.b - start_color.b) * fraction;
}