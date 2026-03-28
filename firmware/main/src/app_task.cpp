#include "app_task.h"
#include "analytics.h"
#include "button_handling.h"
#include "common.h"
#include "hal/u8g2_esp32_hal.h"
#include "heimdall/action_manager.h"
#include "hermes/hermes.h"
#include "i2c_checker.h"
#include "led_status.h"
#include "mercedes/mercedes.h"
#include "message_manager.h"
#include "my_mqtt_client.h"
#include "persistence_manager.h"
#include "simulator.h"
#include "u8g2_mqtt.h"
#include "wifi_manager.h"

#include <cstring>
#include <driver/i2c.h>
#include <esp_diagnostics.h>
#include <esp_insights.h>
#include <esp_log.h>
#include <esp_mac.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <sdkconfig.h>
#include <u8g2.h>

#define PIN_RST GPIO_NUM_NC

static const char *TAG = "app_task";

u8g2_t u8g2;
uint8_t received_signal;
uint64_t last_mqtt_sync = 0;

persistence_manager_t g_persistence_manager;

extern QueueHandle_t buttonQueue;

static TaskHandle_t display_update_task_handle = nullptr;

// Display update task - handles I2C transfer asynchronously
static void display_update_task(void *args)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        u8g2_SendBuffer(&u8g2);
    }
}

static void setup_screen(void)
{
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.bus.i2c.sda = I2C_MASTER_SDA_PIN;
    u8g2_esp32_hal.bus.i2c.scl = I2C_MASTER_SCL_PIN;
    u8g2_esp32_hal.reset = PIN_RST;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb);
    u8x8_SetI2CAddress(&u8g2.u8x8, DISPLAY_I2C_ADDRESS << 1);

    ESP_DIAG_EVENT(TAG, "u8g2_InitDisplay");
    u8g2_InitDisplay(&u8g2);
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_DIAG_EVENT(TAG, "u8g2_SetPowerSave");
    u8g2_SetPowerSave(&u8g2, 0);
    vTaskDelay(pdMS_TO_TICKS(10));

    u8g2_ClearDisplay(&u8g2);
}

// --- Heimdall button action callbacks ---

static void on_button_up(const char *)
{
    Mercedes::getInstance().handleInput(BTN_UP);
}
static void on_button_down(const char *)
{
    Mercedes::getInstance().handleInput(BTN_DOWN);
}
static void on_button_left(const char *)
{
    Mercedes::getInstance().handleInput(BTN_LEFT);
}
static void on_button_right(const char *)
{
    Mercedes::getInstance().handleInput(BTN_RIGHT);
}
static void on_button_select(const char *)
{
    Mercedes::getInstance().handleInput(BTN_SELECT);
}
static void on_button_back(const char *)
{
    Mercedes::getInstance().handleInput(BTN_BACK);
}

static void register_button_actions(void)
{
    action_manager_register("button_up", on_button_up);
    action_manager_register("button_down", on_button_down);
    action_manager_register("button_left", on_button_left);
    action_manager_register("button_right", on_button_right);
    action_manager_register("button_select", on_button_select);
    action_manager_register("button_back", on_button_back);
    ESP_LOGI(TAG, "Button actions registered with Heimdall");
}

// --- Physical button handler → Heimdall ---

static void handle_button(uint8_t button)
{
    hermes_reset_inactivity();

    switch (button)
    {
    case CONFIG_BUTTON_UP:
        action_manager_execute("button_up", NULL);
        break;
    case CONFIG_BUTTON_DOWN:
        action_manager_execute("button_down", NULL);
        break;
    case CONFIG_BUTTON_LEFT:
        action_manager_execute("button_left", NULL);
        break;
    case CONFIG_BUTTON_RIGHT:
        action_manager_execute("button_right", NULL);
        break;
    case CONFIG_BUTTON_BACK:
        action_manager_execute("button_back", NULL);
        break;
    case CONFIG_BUTTON_SELECT:
        action_manager_execute("button_select", NULL);
        break;
    default:
        ESP_LOGE(TAG, "Unhandled button: %u", button);
        break;
    }
}

// --- Message manager listener ---

static void on_message_received(const message_t *msg)
{
    if (!msg || msg->type != MESSAGE_TYPE_SETTINGS)
    {
        return;
    }

    if (std::strcmp(msg->data.settings.key, "light_variant") == 0)
    {
        char val[8];
        snprintf(val, sizeof(val), "%d", (int)msg->data.settings.value.int_value);
        Mercedes::getInstance().updateItemValue("light_variant", val);
        start_simulation_with_reload(true);
        return;
    }

    if (std::strcmp(msg->data.settings.key, "light_mode") == 0)
    {
        char val[8];
        snprintf(val, sizeof(val), "%d", (int)msg->data.settings.value.int_value);
        Mercedes::getInstance().updateItemValue("light_mode", val);
        bool force_reload = (msg->data.settings.type == SETTINGS_TYPE_INT && msg->data.settings.value.int_value == 0);
        start_simulation_with_reload(force_reload);
        return;
    }

    if (std::strcmp(msg->data.settings.key, "light_active") == 0)
    {
        Mercedes::getInstance().updateItemValue("light_active", msg->data.settings.value.bool_value ? "true" : "false");
        start_simulation_with_reload(false);
    }
}

// --- Main task ---

void app_task(void *args)
{
    if (i2c_bus_scan_and_check() != ESP_OK)
    {
        led_behavior_t led_behavior = {
            .on_time_ms = 1000,
            .off_time_ms = 500,
            .color = {.red = 50, .green = 0, .blue = 0},
            .index = 0,
            .mode = LED_MODE_BLINK,
        };
        led_status_set_behavior(led_behavior);

        ESP_LOGE(TAG, "Display not found on I2C bus");
        vTaskDelete(nullptr);
        return;
    }

    setup_screen();

    // Factory reset check (hold BACK button for 5 seconds)
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_BACK);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    vTaskDelay(pdMS_TO_TICKS(10));
    if (gpio_get_level(BUTTON_BACK) == 0)
    {
        u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
        for (int i = 5; i > 0; --i)
        {
            u8g2_ClearBuffer(&u8g2);
            u8g2_DrawStr(&u8g2, 5, 20, "BACK gedrueckt!");
            u8g2_DrawStr(&u8g2, 5, 35, "Halte fuer Reset...");
            char buf[32];
            snprintf(buf, sizeof(buf), "Loesche in %d s", i);
            u8g2_DrawStr(&u8g2, 5, 55, buf);
            u8g2_SendBuffer(&u8g2);
            vTaskDelay(pdMS_TO_TICKS(1000));
            if (gpio_get_level(BUTTON_BACK) != 0)
                break;
            if (i == 1)
            {
                u8g2_ClearBuffer(&u8g2);
                u8g2_DrawStr(&u8g2, 5, 30, "Alle Einstellungen ");
                u8g2_DrawStr(&u8g2, 5, 45, "werden geloescht...");
                u8g2_SendBuffer(&u8g2);
                persistence_manager_factory_reset();
                vTaskDelay(pdMS_TO_TICKS(1000));
                u8g2_ClearBuffer(&u8g2);
                u8g2_DrawStr(&u8g2, 5, 35, "Fertig. Neustart...");
                u8g2_SendBuffer(&u8g2);
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            }
        }
    }

    // Initialize subsystems
    persistence_manager_init(&g_persistence_manager, "config");
    message_manager_init();
    setup_buttons();

    // Initialize Heimdall button actions
    register_button_actions();

    // Initialize Hermes renderer (60s screensaver timeout)
    hermes_init(&u8g2, 60000);

    // Show splash screen immediately
    u8g2_ClearBuffer(&u8g2);
    hermes_draw(0);
    u8g2_SendBuffer(&u8g2);

    // Start network and services
    wifi_manager_init();
    mqtt_client_start();
    message_manager_register_listener(on_message_received);
    start_simulation();

    // Set up dynamic value provider for label items
    {
        // Cache MAC suffix once
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        static char mac_suffix[6];
        snprintf(mac_suffix, sizeof(mac_suffix), "%02X%02X", mac[4], mac[5]);
        ESP_LOGI(TAG, "Device MAC suffix: %s", mac_suffix);

        Mercedes::getInstance().setItemValueProvider([](const std::string &id, char *buf, size_t bufSize) {
            if (id == "mac_suffix")
            {
                strncpy(buf, mac_suffix, bufSize - 1);
                buf[bufSize - 1] = '\0';
            }
        });
    }

    // Load dynamic menu from SPIFFS
    {
        FILE *f = fopen("/spiffs/menu.json", "r");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);
            std::string json(size, '\0');
            fread(&json[0], 1, size, f);
            fclose(f);
            if (Mercedes::getInstance().buildFromJson(json))
            {
                ESP_LOGI(TAG, "Menu loaded from /spiffs/menu.json");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to parse menu.json");
            }
        }
        else
        {
            ESP_LOGE(TAG, "Failed to open /spiffs/menu.json");
        }
    }

    display_mqtt_queue = xQueueCreate(1, 1024);
    xTaskCreatePinnedToCore(u8g2_mqtt_task, "mqtt_disp", 4096, nullptr, 5, nullptr, tskNO_AFFINITY);

    xTaskCreatePinnedToCore(display_update_task, "display_update", 4096, nullptr, tskIDLE_PRIORITY + 1,
                            &display_update_task_handle, CONFIG_FREERTOS_NUMBER_OF_CORES - 1);

    // Main loop
    auto oldTime = esp_timer_get_time();

    while (true)
    {
        auto currentTime = esp_timer_get_time();
        uint64_t deltaMs = (currentTime - oldTime) / 1000;
        oldTime = currentTime;

        u8g2_ClearBuffer(&u8g2);
        hermes_draw(deltaMs);

        // MQTT display sync
        auto now = esp_timer_get_time();
        if (now - last_mqtt_sync > 1000000)
        {
            uint8_t *u8g2_buf = u8g2_GetBufferPtr(&u8g2);
            xQueueOverwrite(display_mqtt_queue, u8g2_buf);
            last_mqtt_sync = now;
        }

        // Signal display task
        if (display_update_task_handle != nullptr)
        {
            xTaskNotifyGive(display_update_task_handle);
        }

        // Process button input
        if (xQueueReceive(buttonQueue, &received_signal, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            handle_button(received_signal);
        }
    }

    cleanup_buttons();
}
