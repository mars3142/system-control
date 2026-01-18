#include "app_task.h"

#include "analytics.h"
#include "button_handling.h"
#include "common.h"
#include "common/InactivityTracker.h"
#include "hal/u8g2_esp32_hal.h"
#include "i2c_checker.h"
#include "led_status.h"
#include "message_manager.h"
#include "persistence_manager.h"
#include "simulator.h"
#include "ui/ClockScreenSaver.h"
#include "ui/ScreenSaver.h"
#include "ui/SplashScreen.h"
#include "wifi_manager.h"
#include <cstring>
#include <driver/i2c.h>
#include <esp_diagnostics.h>
#include <esp_insights.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <sdkconfig.h>
#include <u8g2.h>

#define PIN_RST GPIO_NUM_NC

static const char *TAG = "app_task";

u8g2_t u8g2;
uint8_t last_value = 0;
menu_options_t options;
uint8_t received_signal;

std::shared_ptr<Widget> m_widget;
std::vector<std::shared_ptr<Widget>> m_history;
std::unique_ptr<InactivityTracker> m_inactivityTracker;
// Persistence Manager für C-API
persistence_manager_t g_persistence_manager;

extern QueueHandle_t buttonQueue;

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

void setScreen(const std::shared_ptr<Widget> &screen)
{
    if (screen != nullptr)
    {
        ESP_DIAG_EVENT(TAG, "Screen set: %s", screen->getName());
        m_widget = screen;
        m_history.clear();
        m_history.emplace_back(m_widget);
        m_widget->onEnter();
    }
}

void pushScreen(const std::shared_ptr<Widget> &screen)
{
    if (screen != nullptr)
    {
        if (m_widget)
        {
            m_widget->onPause();
        }
        ESP_DIAG_EVENT(TAG, "Screen pushed: %s", screen->getName());
        m_widget = screen;
        m_widget->onEnter();
        m_history.emplace_back(m_widget);
    }
}

void popScreen()
{
    if (m_history.size() >= 2)
    {
        m_history.pop_back();
        if (m_widget)
        {
            persistence_manager_save(&g_persistence_manager);
            m_widget->onExit();
        }
        m_widget = m_history.back();
        ESP_DIAG_EVENT(TAG, "Screen popped, now: %s", m_widget->getName());
        m_widget->onResume();
    }
}

static void init_ui(void)
{
    persistence_manager_init(&g_persistence_manager, "config");
    options = {
        .u8g2 = &u8g2,
        .setScreen = [](const std::shared_ptr<Widget> &screen) { setScreen(screen); },
        .pushScreen = [](const std::shared_ptr<Widget> &screen) { pushScreen(screen); },
        .popScreen = []() { popScreen(); },
        .onButtonClicked = nullptr,
        .persistenceManager = &g_persistence_manager,
    };
    m_widget = std::make_shared<SplashScreen>(&options);
    m_inactivityTracker = std::make_unique<InactivityTracker>(60000, []() {
        auto screensaver = std::make_shared<ClockScreenSaver>(&options);
        options.pushScreen(screensaver);
    });

    u8g2_ClearBuffer(&u8g2);
    m_widget->Render();
    u8g2_SendBuffer(&u8g2);
}

static void on_message_received(const message_t *msg)
{
    if (msg && msg->type == MESSAGE_TYPE_SETTINGS && msg->data.settings.type == SETTINGS_TYPE_BOOL &&
        std::strcmp(msg->data.settings.key, "light_active") == 0)
    {
        start_simulation();
    }
}

static void handle_button(uint8_t button)
{
    m_inactivityTracker->reset();

    if (m_widget)
    {
        switch (button)
        {
        case 1:
            m_widget->OnButtonClicked(ButtonType::UP);
            break;

        case 3:
            m_widget->OnButtonClicked(ButtonType::LEFT);
            break;

        case 5:
            m_widget->OnButtonClicked(ButtonType::RIGHT);
            break;

        case 6:
            m_widget->OnButtonClicked(ButtonType::DOWN);
            break;

        case 16:
            m_widget->OnButtonClicked(ButtonType::BACK);
            break;

        case 18:
            m_widget->OnButtonClicked(ButtonType::SELECT);
            break;

        default:
            ESP_LOGE(TAG, "Unhandled button: %u", button);
            break;
        }
    }
}

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

    // Display initialisieren, damit Info angezeigt werden kann
    setup_screen();

    // BACK-Button prüfen und ggf. Einstellungen löschen (mit Countdown)
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
            {
                // Button losgelassen, abbrechen
                break;
            }
            if (i == 1)
            {
                // After 5 seconds still pressed: perform factory reset
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

    message_manager_init();

    setup_buttons();
    init_ui();

    wifi_manager_init();

    message_manager_register_listener(on_message_received);

    start_simulation();

    auto oldTime = esp_timer_get_time();

    while (true)
    {
        u8g2_ClearBuffer(&u8g2);

        if (m_widget != nullptr)
        {
            auto currentTime = esp_timer_get_time();
            auto delta = currentTime - oldTime;
            oldTime = currentTime;

            uint64_t deltaMs = delta / 1000;

            m_widget->Update(deltaMs);
            m_widget->Render();

            m_inactivityTracker->update(deltaMs);
        }

        u8g2_SendBuffer(&u8g2);

        if (xQueueReceive(buttonQueue, &received_signal, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            handle_button(received_signal);
        }
    }

    cleanup_buttons();
}
