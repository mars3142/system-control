#pragma once

#include "esp_check.h"
#include <stdint.h>

esp_err_t add_light_item(const char time[5], uint8_t red, uint8_t green, uint8_t blue);

void cleanup_light_items(void);

#ifdef __cplusplus
extern "C"
{
#endif
    void simulate(void *args);
#ifdef __cplusplus
}
#endif
