#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hal/u8g2_esp32_hal.h"

static const char *TAG = "u8g2_hal";
static const unsigned int I2C_TIMEOUT_MS = 1000;

static spi_device_handle_t handle_spi;   // SPI handle.
static i2c_master_bus_handle_t i2c_bus;  // I2C bus handle (new driver).
static i2c_master_dev_handle_t i2c_dev;  // I2C device handle (new driver).
static uint8_t i2c_tx_buf[256];          // Buffer for one I2C transaction.
static size_t i2c_tx_len;                // Current length in buffer.
static uint8_t current_i2c_addr7;        // Current 7-bit device address.
static u8g2_esp32_hal_t u8g2_esp32_hal;  // HAL state data.
static bool i2c_transfer_failed = false; // Flag to track I2C transfer errors

#define HOST SPI2_HOST

#undef ESP_ERROR_CHECK
#define ESP_ERROR_CHECK(x)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        esp_err_t rc = (x);                                                                                            \
        if (rc != ESP_OK)                                                                                              \
        {                                                                                                              \
            ESP_LOGE("err", "esp_err_t = %d", rc);                                                                     \
            assert(0 && #x);                                                                                           \
        }                                                                                                              \
    } while (0);

// Softer error handling for I2C operations that may fail temporarily
#define I2C_ERROR_CHECK(x)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        esp_err_t rc = (x);                                                                                            \
        if (rc != ESP_OK)                                                                                              \
        {                                                                                                              \
            ESP_LOGW(TAG, "I2C error: %s = %d", #x, rc);                                                               \
            i2c_transfer_failed = true;                                                                                \
        }                                                                                                              \
    } while (0);

/*
 * Initialze the ESP32 HAL.
 */
void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param)
{
    u8g2_esp32_hal = u8g2_esp32_hal_param;
} // u8g2_esp32_hal_init

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is
 * invoked to handle SPI communications.
 */
uint8_t u8g2_esp32_spi_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    ESP_LOGD(TAG, "spi_byte_cb: Received a msg: %d, arg_int: %d, arg_ptr: %p", msg, arg_int, arg_ptr);
    switch (msg)
    {
    case U8X8_MSG_BYTE_SET_DC:
        if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED)
        {
            gpio_set_level(u8g2_esp32_hal.dc, arg_int);
        }
        break;

    case U8X8_MSG_BYTE_INIT: {
        if (u8g2_esp32_hal.bus.spi.clk == U8G2_ESP32_HAL_UNDEFINED ||
            u8g2_esp32_hal.bus.spi.mosi == U8G2_ESP32_HAL_UNDEFINED ||
            u8g2_esp32_hal.bus.spi.cs == U8G2_ESP32_HAL_UNDEFINED)
        {
            break;
        }

        spi_bus_config_t bus_config;
        memset(&bus_config, 0, sizeof(spi_bus_config_t));
        bus_config.sclk_io_num = u8g2_esp32_hal.bus.spi.clk;  // CLK
        bus_config.mosi_io_num = u8g2_esp32_hal.bus.spi.mosi; // MOSI
        bus_config.miso_io_num = GPIO_NUM_NC;                 // MISO
        bus_config.quadwp_io_num = GPIO_NUM_NC;               // Not used
        bus_config.quadhd_io_num = GPIO_NUM_NC;               // Not used
        // ESP_LOGI(TAG, "... Initializing bus.");
        ESP_ERROR_CHECK(spi_bus_initialize(HOST, &bus_config, 1));

        spi_device_interface_config_t dev_config;
        dev_config.address_bits = 0;
        dev_config.command_bits = 0;
        dev_config.dummy_bits = 0;
        dev_config.mode = 0;
        dev_config.duty_cycle_pos = 0;
        dev_config.cs_ena_posttrans = 0;
        dev_config.cs_ena_pretrans = 0;
        dev_config.clock_speed_hz = 10000;
        dev_config.spics_io_num = u8g2_esp32_hal.bus.spi.cs;
        dev_config.flags = 0;
        dev_config.queue_size = 200;
        dev_config.pre_cb = NULL;
        dev_config.post_cb = NULL;
        // ESP_LOGI(TAG, "... Adding device bus.");
        ESP_ERROR_CHECK(spi_bus_add_device(HOST, &dev_config, &handle_spi));

        break;
    }

    case U8X8_MSG_BYTE_SEND: {
        spi_transaction_t trans_desc;
        trans_desc.addr = 0;
        trans_desc.cmd = 0;
        trans_desc.flags = 0;
        trans_desc.length = 8 * arg_int; // Number of bits NOT number of bytes.
        trans_desc.rxlength = 0;
        trans_desc.tx_buffer = arg_ptr;
        trans_desc.rx_buffer = NULL;

        // ESP_LOGI(TAG, "... Transmitting %d bytes.", arg_int);
        ESP_ERROR_CHECK(spi_device_transmit(handle_spi, &trans_desc));
        break;
    }
    }
    return 0;
} // u8g2_esp32_spi_byte_cb

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is
 * invoked to handle I2C communications.
 */
uint8_t u8g2_esp32_i2c_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    ESP_LOGD(TAG, "i2c_cb: Received a msg: %d, arg_int: %d, arg_ptr: %p", msg, arg_int, arg_ptr);

    switch (msg)
    {
    case U8X8_MSG_BYTE_SET_DC: {
        if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED)
        {
            gpio_set_level(u8g2_esp32_hal.dc, arg_int);
        }
        break;
    }

    case U8X8_MSG_BYTE_INIT: {
        if (u8g2_esp32_hal.bus.i2c.sda == U8G2_ESP32_HAL_UNDEFINED ||
            u8g2_esp32_hal.bus.i2c.scl == U8G2_ESP32_HAL_UNDEFINED)
        {
            break;
        }

        // Neue I2C-Master-API: Bus einmalig anlegen
        if (i2c_bus == NULL)
        {
            i2c_master_bus_config_t bus_cfg = {
                .i2c_port = I2C_MASTER_NUM,
                .scl_io_num = u8g2_esp32_hal.bus.i2c.scl,
                .sda_io_num = u8g2_esp32_hal.bus.i2c.sda,
                .clk_source = I2C_CLK_SRC_DEFAULT,
                .flags = {.enable_internal_pullup = true},
            };

            ESP_LOGI(TAG, "sda_io_num %d", u8g2_esp32_hal.bus.i2c.sda);
            ESP_LOGI(TAG, "scl_io_num %d", u8g2_esp32_hal.bus.i2c.scl);
            ESP_LOGI(TAG, "clk_speed %d", I2C_MASTER_FREQ_HZ);
            ESP_LOGI(TAG, "i2c_new_master_bus %d", I2C_MASTER_NUM);
            ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &i2c_bus));
        }
        break;
    }

    case U8X8_MSG_BYTE_SEND: {
        if (i2c_transfer_failed)
        {
            break; // Skip sending if transfer already failed
        }
        uint8_t *data_ptr = (uint8_t *)arg_ptr;
        ESP_LOG_BUFFER_HEXDUMP(TAG, data_ptr, arg_int, ESP_LOG_VERBOSE);

        // Bytes in lokalen Puffer sammeln, tatsächliche Übertragung bei END_TRANSFER
        if (i2c_tx_len + (size_t)arg_int > sizeof(i2c_tx_buf))
        {
            ESP_LOGW(TAG, "I2C tx buffer overflow (%zu + %d)", i2c_tx_len, arg_int);
            i2c_transfer_failed = true;
            break;
        }
        memcpy(&i2c_tx_buf[i2c_tx_len], data_ptr, arg_int);
        i2c_tx_len += (size_t)arg_int;
        break;
    }

    case U8X8_MSG_BYTE_START_TRANSFER: {
        uint8_t i2c_address = u8x8_GetI2CAddress(u8x8);
        ESP_LOGD(TAG, "Start I2C transfer to %02X.", i2c_address >> 1);
        i2c_transfer_failed = false; // Reset error flag at start of transfer

        // Für neuen Treiber: Device-Handle für diese 7-Bit-Adresse anlegen (oder wiederverwenden)
        uint8_t addr7 = i2c_address >> 1;
        if (i2c_dev == NULL || addr7 != current_i2c_addr7)
        {
            if (i2c_dev)
            {
                i2c_master_bus_rm_device(i2c_dev);
                i2c_dev = NULL;
            }

            i2c_device_config_t dev_cfg = {
                .device_address = addr7,
                .scl_speed_hz = I2C_MASTER_FREQ_HZ,
            };
            ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev));
            current_i2c_addr7 = addr7;
        }
        i2c_tx_len = 0;
        break;
    }

    case U8X8_MSG_BYTE_END_TRANSFER: {
        ESP_LOGD(TAG, "End I2C transfer.");
        if (!i2c_transfer_failed && i2c_dev != NULL && i2c_tx_len > 0)
        {
            esp_err_t rc = i2c_master_transmit(i2c_dev, i2c_tx_buf, i2c_tx_len, I2C_TIMEOUT_MS);
            if (rc != ESP_OK)
            {
                ESP_LOGW(TAG, "I2C error: i2c_master_transmit = %d", rc);
                i2c_transfer_failed = true;
            }
        }
        break;
    }
    }
    return 0;
} // u8g2_esp32_i2c_byte_cb

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is
 * invoked to handle callbacks for GPIO and delay functions.
 */
uint8_t u8g2_esp32_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    ESP_LOGD(TAG, "gpio_and_delay_cb: Received a msg: %d, arg_int: %d, arg_ptr: %p", msg, arg_int, arg_ptr);

    switch (msg)
    {
        // Initialize the GPIO and DELAY HAL functions.  If the pins for DC and
        // RESET have been specified then we define those pins as GPIO outputs.
    case U8X8_MSG_GPIO_AND_DELAY_INIT: {
        uint64_t bitmask = 0;
        if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED)
        {
            bitmask = bitmask | (1ull << u8g2_esp32_hal.dc);
        }
        if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED)
        {
            bitmask = bitmask | (1ull << u8g2_esp32_hal.reset);
        }
        if (u8g2_esp32_hal.bus.spi.cs != U8G2_ESP32_HAL_UNDEFINED)
        {
            bitmask = bitmask | (1ull << u8g2_esp32_hal.bus.spi.cs);
        }

        if (bitmask == 0)
        {
            break;
        }
        gpio_config_t gpioConfig;
        gpioConfig.pin_bit_mask = bitmask;
        gpioConfig.mode = GPIO_MODE_OUTPUT;
        gpioConfig.pull_up_en = GPIO_PULLUP_DISABLE;
        gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
        gpioConfig.intr_type = GPIO_INTR_DISABLE;
        gpio_config(&gpioConfig);
        break;
    }

        // Set the GPIO reset pin to the value passed in through arg_int.
    case U8X8_MSG_GPIO_RESET:
        if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED)
        {
            gpio_set_level(u8g2_esp32_hal.reset, arg_int);
        }
        break;
        // Set the GPIO client select pin to the value passed in through arg_int.
    case U8X8_MSG_GPIO_CS:
        if (u8g2_esp32_hal.bus.spi.cs != U8G2_ESP32_HAL_UNDEFINED)
        {
            gpio_set_level(u8g2_esp32_hal.bus.spi.cs, arg_int);
        }
        break;
        // Set the Software I²C pin to the value passed in through arg_int.
    case U8X8_MSG_GPIO_I2C_CLOCK:
        if (u8g2_esp32_hal.bus.i2c.scl != U8G2_ESP32_HAL_UNDEFINED)
        {
            gpio_set_level(u8g2_esp32_hal.bus.i2c.scl, arg_int);
            //				printf("%c",(arg_int==1?'C':'c'));
        }
        break;
        // Set the Software I²C pin to the value passed in through arg_int.
    case U8X8_MSG_GPIO_I2C_DATA:
        if (u8g2_esp32_hal.bus.i2c.sda != U8G2_ESP32_HAL_UNDEFINED)
        {
            gpio_set_level(u8g2_esp32_hal.bus.i2c.sda, arg_int);
            //				printf("%c",(arg_int==1?'D':'d'));
        }
        break;

        // Delay for the number of milliseconds passed in through arg_int.
    case U8X8_MSG_DELAY_MILLI:
        vTaskDelay(arg_int / portTICK_PERIOD_MS);
        break;
    }
    return 0;
} // u8g2_esp32_gpio_and_delay_cb
