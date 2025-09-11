#pragma once

#include "esp_err.h"

#define DISPLAY_I2C_ADDRESS 0x3C

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define I2C_MASTER_SDA_PIN GPIO_NUM_35
#define I2C_MASTER_SCL_PIN GPIO_NUM_36
#else
/// just dummy pins, because of compile check
#define I2C_MASTER_SDA_PIN GPIO_NUM_20
#define I2C_MASTER_SCL_PIN GPIO_NUM_21
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    esp_err_t i2c_bus_scan_and_check(void);
#ifdef __cplusplus
}
#endif
