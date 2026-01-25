#pragma once

#include "driver/gpio.h"
#include "esp_err.h"

#define DISPLAY_I2C_ADDRESS 0x3C

#define I2C_MASTER_SDA_PIN ((gpio_num_t)CONFIG_DISPLAY_SDA_PIN)
#define I2C_MASTER_SCL_PIN ((gpio_num_t)CONFIG_DISPLAY_SCL_PIN)

__BEGIN_DECLS
esp_err_t i2c_bus_scan_and_check(void);
__END_DECLS
