#include "i2c_checker.h"

#include "driver/i2c_master.h"
#include "esp_insights.h"
#include "esp_log.h"
#include "hal/u8g2_esp32_hal.h"

static const char *TAG = "i2c_checker";

static esp_err_t i2c_device_check(i2c_master_bus_handle_t i2c_bus, uint8_t device_address)
{
    // Use the new I2C master driver to probe for the device.
    return i2c_master_probe(i2c_bus, device_address, 100);
}

esp_err_t i2c_bus_scan_and_check(void)
{
    // 1. Configure and create I2C master bus using the new driver API
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_PIN,
        .sda_io_num = I2C_MASTER_SDA_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags = {.enable_internal_pullup = true},
    };

    esp_err_t err = i2c_new_master_bus(&bus_cfg, &i2c_bus);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C bus creation failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "I2C master bus initialized. Searching for device...");

    // 2. Check if the device is present using the new API
    err = i2c_device_check(i2c_bus, DISPLAY_I2C_ADDRESS);

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

    // 3. Delete I2C master bus if it is no longer needed
    esp_err_t del_err = i2c_del_master_bus(i2c_bus);
    if (del_err != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to delete I2C master bus: %s", esp_err_to_name(del_err));
    }
    ESP_DIAG_EVENT(TAG, "I2C master bus deleted.");

    return err;
}
