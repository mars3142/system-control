#pragma once

#include "driver/i2c.h"

void IRAM_ATTR button_isr_handler(void* arg);

#ifdef __cplusplus
extern "C" {
#endif
void setupButtons(void);
#ifdef __cplusplus
}
#endif