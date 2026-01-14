#include "color.h"
#include <math.h>

rgb_t interpolate_color_rgb(rgb_t start, rgb_t end, float factor)
{
    // Clamp factor to [0, 1]
    if (factor > 1.0f)
        factor = 1.0f;
    if (factor < 0.0f)
        factor = 0.0f;

    rgb_t result;
    result.red = (uint8_t)(start.red + (end.red - start.red) * factor);
    result.green = (uint8_t)(start.green + (end.green - start.green) * factor);
    result.blue = (uint8_t)(start.blue + (end.blue - start.blue) * factor);

    return result;
}

rgb_t interpolate_color_hsv(rgb_t start, rgb_t end, float factor)
{
    // Clamp factor to [0, 1]
    if (factor > 1.0f)
        factor = 1.0f;
    if (factor < 0.0f)
        factor = 0.0f;

    // Convert RGB to HSV
    hsv_t start_hsv = rgb_to_hsv(start);
    hsv_t end_hsv = rgb_to_hsv(end);

    // Handle hue interpolation carefully (circular)
    double h1 = start_hsv.h;
    double h2 = end_hsv.h;
    double diff = h2 - h1;

    if (diff > 180.0)
    {
        h1 += 360.0;
    }
    else if (diff < -180.0)
    {
        h2 += 360.0;
    }

    // Interpolate HSV values
    hsv_t interpolated_hsv;
    interpolated_hsv.h = fmodf(h1 + (h2 - h1) * factor, 360.0f);
    if (interpolated_hsv.h < 0.0f)
    {
        interpolated_hsv.h += 360.0f;
    }
    interpolated_hsv.s = start_hsv.s + (end_hsv.s - start_hsv.s) * factor;
    interpolated_hsv.v = start_hsv.v + (end_hsv.v - start_hsv.v) * factor;

    // Convert back to RGB
    return hsv_to_rgb(interpolated_hsv);
}

hsv_t rgb_to_hsv(rgb_t rgb)
{
    hsv_t hsv;
    uint8_t max = rgb.red;
    uint8_t min = rgb.red;

    if (rgb.green > max)
        max = rgb.green;
    if (rgb.blue > max)
        max = rgb.blue;
    if (rgb.green < min)
        min = rgb.green;
    if (rgb.blue < min)
        min = rgb.blue;

    uint8_t delta = max - min;

    // Value berechnen
    hsv.v = max;

    // Saturation berechnen
    if (max != 0)
    {
        hsv.s = (delta * 255) / max;
    }
    else
    {
        // Schwarz (r = g = b = 0)
        hsv.s = 0;
        hsv.h = 0;
        return hsv;
    }

    // Hue berechnen
    if (delta != 0)
    {
        int16_t hue;

        if (rgb.red == max)
        {
            // Zwischen Gelb und Magenta
            hue = ((int16_t)(rgb.green - rgb.blue) * 30) / delta;
            if (hue < 0)
                hue += 180;
        }
        else if (rgb.green == max)
        {
            // Zwischen Cyan und Gelb
            hue = 60 + ((int16_t)(rgb.blue - rgb.red) * 30) / delta;
        }
        else
        {
            // Zwischen Magenta und Cyan
            hue = 120 + ((int16_t)(rgb.red - rgb.green) * 30) / delta;
        }

        hsv.h = (uint8_t)hue;
    }
    else
    {
        // Graustufe
        hsv.h = 0;
    }

    return hsv;
}

rgb_t hsv_to_rgb(hsv_t hsv)
{
    rgb_t rgb;

    if (hsv.s == 0)
    {
        // Graustufe
        rgb.red = hsv.v;
        rgb.green = hsv.v;
        rgb.blue = hsv.v;
    }
    else
    {
        uint16_t region = hsv.h / 30;
        uint16_t remainder = (hsv.h - (region * 30)) * 6;

        uint8_t p = (hsv.v * (255 - hsv.s)) / 255;
        uint8_t q = (hsv.v * (255 - ((hsv.s * remainder) / 180))) / 255;
        uint8_t t = (hsv.v * (255 - ((hsv.s * (180 - remainder)) / 180))) / 255;

        switch (region)
        {
        case 0:
            rgb.red = hsv.v;
            rgb.green = t;
            rgb.blue = p;
            break;
        case 1:
            rgb.red = q;
            rgb.green = hsv.v;
            rgb.blue = p;
            break;
        case 2:
            rgb.red = p;
            rgb.green = hsv.v;
            rgb.blue = t;
            break;
        case 3:
            rgb.red = p;
            rgb.green = q;
            rgb.blue = hsv.v;
            break;
        case 4:
            rgb.red = t;
            rgb.green = p;
            rgb.blue = hsv.v;
            break;
        default: // case 5:
            rgb.red = hsv.v;
            rgb.green = p;
            rgb.blue = q;
            break;
        }
    }

    return rgb;
}
