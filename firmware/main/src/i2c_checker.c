#include "i2c_checker.h"

#include "driver/i2c.h"
#include "esp_insights.h"
#include "esp_log.h"
#include "hal/u8g2_esp32_hal.h"

static const char *TAG = "i2c_checker";

esp_err_t i2c_device_check(i2c_port_t i2c_port, uint8_t device_address)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Send the device address with the write bit (LSB = 0)
    i2c_master_write_byte(cmd, (device_address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, pdMS_TO_TICKS(100));

    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t i2c_bus_scan_and_check(void)
{
    // 1. Configure and install I2C bus
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_PIN,
        .scl_io_num = I2C_MASTER_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C parameter configuration failed: %s", esp_err_to_name(err));
        return err;
    }

    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C driver installation failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C driver initialized. Searching for device...");

    // 2. Check if the device is present
    err = i2c_device_check(I2C_MASTER_NUM, DISPLAY_I2C_ADDRESS);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Device found at address 0x%02X!", DISPLAY_I2C_ADDRESS);
        // Here you could now call e.g. setup_screen()
    }
    else if (err == ESP_ERR_TIMEOUT)
    {
        ESP_LOGE(TAG, "Timeout! Device at address 0x%02X is not responding.", DISPLAY_I2C_ADDRESS);
    }
    else
    {
        ESP_LOGE(TAG, "Error communicating with address 0x%02X: %s", DISPLAY_I2C_ADDRESS, esp_err_to_name(err));
    }

    // 3. Uninstall I2C driver if it is no longer needed
    i2c_driver_delete(I2C_MASTER_NUM);
    ESP_DIAG_EVENT(TAG, "I2C driver uninstalled.");

    return err;
}
