#include "app_task.h"
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

#if defined(CONFIG_IRIS_ENABLED)
#include "iris/iris.h"
#endif

#include <cstring>
#include <driver/i2c.h>
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

    ESP_LOGI(TAG, "u8g2_InitDisplay");
    u8g2_InitDisplay(&u8g2);
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_LOGI(TAG, "u8g2_SetPowerSave");
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

// =============================================================================
// Iris dynamic screen builders
// Called by the DynamicScreenProvider when navigating to dynamic screens.
// =============================================================================

#if defined(CONFIG_IRIS_ENABLED)

// Helper: build a capability detail screen for a paired device.
// Master + online → toggles/selection; offline or Backup → read-only labels.
static void build_paired_device_screen(const iris_device_t &dev)
{
    char eui_str[17];
    iris_eui64_to_str(dev.p.eui64, eui_str, sizeof(eui_str));

    MenuScreenDef screen;
    screen.id    = std::string("iris_dev_") + eui_str;
    screen.title = dev.p.name;
    screen.dynamic = false;

    bool interactive = iris_is_master() && dev.online;

    if (dev.p.capabilities & IRIS_CAP_INNER_LIGHT)
    {
        MenuItemDef item;
        item.id    = std::string("tgl_inner_") + eui_str;
        item.label = "Innenbeleuchtung";
        if (interactive)
        {
            item.type        = "toggle";
            item.toggleValue = (dev.p.state & IRIS_STATE_INNER_LIGHT) != 0;
            item.actionTopic = std::string("iris/toggle/") + eui_str + "/inner";
        }
        else
        {
            item.type  = "label";
            item.label += std::string(": ") + ((dev.p.state & IRIS_STATE_INNER_LIGHT) ? "an" : "aus");
        }
        screen.items.push_back(item);
    }

    if (dev.p.capabilities & IRIS_CAP_OUTER_LIGHT)
    {
        MenuItemDef item;
        item.id    = std::string("tgl_outer_") + eui_str;
        item.label = "Aussenbeleuchtung";
        if (interactive)
        {
            item.type        = "toggle";
            item.toggleValue = (dev.p.state & IRIS_STATE_OUTER_LIGHT) != 0;
            item.actionTopic = std::string("iris/toggle/") + eui_str + "/outer";
        }
        else
        {
            item.type  = "label";
            item.label += std::string(": ") + ((dev.p.state & IRIS_STATE_OUTER_LIGHT) ? "an" : "aus");
        }
        screen.items.push_back(item);
    }

    if (dev.p.capabilities & IRIS_CAP_MOVEMENT)
    {
        MenuItemDef item;
        item.id    = std::string("sel_move_") + eui_str;
        item.label = "Bewegung";
        if (interactive)
        {
            item.type        = "selection";
            item.actionTopic = std::string("iris/toggle/") + eui_str + "/movement";
            MenuSelectionItemDef oben, unten;
            oben.value  = "1"; oben.label  = "Oben";
            unten.value = "0"; unten.label = "Unten";
            item.selectionItems  = {oben, unten};
            item.selectionIndex  = (dev.p.state & IRIS_STATE_MOVEMENT) ? 0 : 1;
        }
        else
        {
            item.type  = "label";
            item.label += std::string(": ") + ((dev.p.state & IRIS_STATE_MOVEMENT) ? "Oben" : "Unten");
        }
        screen.items.push_back(item);
    }

    if (screen.items.empty())
    {
        MenuItemDef item;
        item.id   = std::string("no_cap_") + eui_str;
        item.type = "label";
        item.label = "Keine Funktionen";
        screen.items.push_back(item);
    }

    // Delete item — available in all states (online/offline, master/backup)
    {
        MenuItemDef item;
        item.id          = std::string("del_dev_") + eui_str;
        item.type        = "action";
        item.label       = "Loeschen";
        item.actionTopic = std::string("iris/unpair/") + eui_str;
        screen.items.push_back(item);
    }

    Mercedes::getInstance().addOrReplaceScreen(screen);
}

// Helper: build the preview + "Aufnehmen" screen for a discovered (unpaired) device.
static void build_new_device_screen(const iris_device_t &dev)
{
    char eui_str[17];
    iris_eui64_to_str(dev.p.eui64, eui_str, sizeof(eui_str));

    MenuScreenDef screen;
    screen.id    = std::string("iris_new_") + eui_str;
    screen.title = dev.p.name;
    screen.dynamic = false;

    auto add_cap_label = [&](const char *label, bool has_cap) {
        MenuItemDef item;
        item.id    = std::string("cap_") + label + "_" + eui_str;
        item.type  = "label";
        item.label = std::string(label) + (has_cap ? ": ja" : ": nein");
        screen.items.push_back(item);
    };
    add_cap_label("Innenbeleuchtung", dev.p.capabilities & IRIS_CAP_INNER_LIGHT);
    add_cap_label("Aussenbeleuchtung", dev.p.capabilities & IRIS_CAP_OUTER_LIGHT);
    add_cap_label("Bewegung",          dev.p.capabilities & IRIS_CAP_MOVEMENT);

    // "Aufnehmen" action
    MenuItemDef pair_item;
    pair_item.id          = std::string("pair_") + eui_str;
    pair_item.type        = "action";
    pair_item.label       = "Aufnehmen";
    pair_item.actionTopic = std::string("iris/pair/") + eui_str;
    screen.items.push_back(pair_item);

    Mercedes::getInstance().addOrReplaceScreen(screen);
}

// Dynamic provider: called when entering lights_menu.
// Adds explicit AN/AUS actions for multicast — not toggle, so that
// "Alle Innen AN" turns everything on regardless of current state.
static void on_dynamic_lights(void)
{
    const char *inner_on_id  = "iris_all_inner_on";
    const char *inner_off_id = "iris_all_inner_off";
    const char *outer_on_id  = "iris_all_outer_on";
    const char *outer_off_id = "iris_all_outer_off";

    if (iris_any_has_cap(IRIS_CAP_INNER_LIGHT))
    {
        MenuItemDef on_item;
        on_item.id          = inner_on_id;
        on_item.type        = "action";
        on_item.label       = "Alle Innen AN";
        on_item.actionTopic = "iris/set_all/inner/on";
        Mercedes::getInstance().ensureItemInScreen("lights_menu", on_item);

        MenuItemDef off_item;
        off_item.id          = inner_off_id;
        off_item.type        = "action";
        off_item.label       = "Alle Innen AUS";
        off_item.actionTopic = "iris/set_all/inner/off";
        Mercedes::getInstance().ensureItemInScreen("lights_menu", off_item);
    }
    else
    {
        Mercedes::getInstance().removeItemFromScreen("lights_menu", inner_on_id);
        Mercedes::getInstance().removeItemFromScreen("lights_menu", inner_off_id);
    }

    if (iris_any_has_cap(IRIS_CAP_OUTER_LIGHT))
    {
        MenuItemDef on_item;
        on_item.id          = outer_on_id;
        on_item.type        = "action";
        on_item.label       = "Alle Aussen AN";
        on_item.actionTopic = "iris/set_all/outer/on";
        Mercedes::getInstance().ensureItemInScreen("lights_menu", on_item);

        MenuItemDef off_item;
        off_item.id          = outer_off_id;
        off_item.type        = "action";
        off_item.label       = "Alle Aussen AUS";
        off_item.actionTopic = "iris/set_all/outer/off";
        Mercedes::getInstance().ensureItemInScreen("lights_menu", off_item);
    }
    else
    {
        Mercedes::getInstance().removeItemFromScreen("lights_menu", outer_on_id);
        Mercedes::getInstance().removeItemFromScreen("lights_menu", outer_off_id);
    }
}

// Dynamic provider: called when entering settings_menu.
// Adds "Geraet hinzufuegen" only when this unit is Master.
static void on_dynamic_settings(void)
{
    const char *add_dev_id = "menu_add_device";

    if (iris_is_master())
    {
        MenuItemDef item;
        item.id            = add_dev_id;
        item.type          = "submenu";
        item.label         = "Geraet hinzufuegen";
        item.targetScreenId = "iris_new_devices_menu";
        Mercedes::getInstance().ensureItemInScreen("settings_menu", item);
    }
    else
    {
        Mercedes::getInstance().removeItemFromScreen("settings_menu", add_dev_id);
    }
}

// Dynamic provider: called when entering external_devices_menu.
static void on_dynamic_external_devices(void)
{
    iris_device_t devices[CONFIG_IRIS_MAX_DEVICES];
    int count = iris_get_paired(devices, CONFIG_IRIS_MAX_DEVICES);

    MenuScreenDef screen;
    screen.id     = "external_devices_menu";
    screen.title  = "externe Geraete";
    screen.dynamic = true;

    if (count == 0)
    {
        MenuItemDef item;
        item.id   = "ext_empty";
        item.type = "label";
        item.label = "keine Eintraege";
        screen.items.push_back(item);
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            char eui_str[17];
            iris_eui64_to_str(devices[i].p.eui64, eui_str, sizeof(eui_str));

            MenuItemDef item;
            item.id            = std::string("ext_dev_") + eui_str;
            item.type          = "submenu";
            // Append [off] suffix for offline devices
            item.label         = std::string(devices[i].p.name) +
                                  (devices[i].online ? "" : " [off]");
            item.targetScreenId = std::string("iris_dev_") + eui_str;
            screen.items.push_back(item);

            // Pre-build capability screen for this device
            build_paired_device_screen(devices[i]);
        }
    }
    Mercedes::getInstance().addOrReplaceScreen(screen);
}

// Dynamic provider: called when entering iris_new_devices_menu.
static void on_dynamic_new_devices(void)
{
    iris_device_t found[8];
    int count = iris_scan(found, 8);

    MenuScreenDef screen;
    screen.id     = "iris_new_devices_menu";
    screen.title  = "Geraet hinzufuegen";
    screen.dynamic = true;

    if (count == 0)
    {
        MenuItemDef item;
        item.id    = "scan_none";
        item.type  = "label";
        item.label = "Keine Geraete";
        screen.items.push_back(item);
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            char eui_str[17];
            iris_eui64_to_str(found[i].p.eui64, eui_str, sizeof(eui_str));

            MenuItemDef item;
            item.id            = std::string("new_dev_") + eui_str;
            item.type          = "submenu";
            item.label         = found[i].p.name;
            item.targetScreenId = std::string("iris_new_") + eui_str;
            screen.items.push_back(item);

            build_new_device_screen(found[i]);
        }
    }
    Mercedes::getInstance().addOrReplaceScreen(screen);
}

// Register the dynamic screen provider after buildFromJson().
static void register_iris_providers(void)
{
    Mercedes::getInstance().setDynamicScreenProvider([](const std::string &screenId) {
        if (screenId == "lights_menu")
            on_dynamic_lights();
        else if (screenId == "settings_menu")
            on_dynamic_settings();
        else if (screenId == "external_devices_menu")
            on_dynamic_external_devices();
        else if (screenId == "iris_new_devices_menu")
            on_dynamic_new_devices();
    });

    Mercedes::getInstance().setActionCallback(
        [](const std::string & /*id*/, const std::string &topic, const std::string &value) {
            // iris/pair/<eui>
            if (topic.rfind("iris/pair/", 0) == 0)
            {
                std::string eui_str = topic.substr(10);
                uint8_t eui64[IRIS_EUI64_LEN];
                if (iris_str_to_eui64(eui_str.c_str(), eui64))
                {
                    const MenuScreenDef *screen = Mercedes::getInstance().getCurrentScreen();
                    const char *name = screen ? screen->title.c_str() : eui_str.c_str();
                    iris_pair(eui64, name);
                    // Refresh external devices screen on next navigation
                }
                return;
            }

            // iris/unpair/<eui>
            if (topic.rfind("iris/unpair/", 0) == 0)
            {
                std::string eui_str = topic.substr(12);
                uint8_t eui64[IRIS_EUI64_LEN];
                if (iris_str_to_eui64(eui_str.c_str(), eui64))
                    iris_unpair(eui64);
                // Navigate back
                Mercedes::getInstance().handleInput(BTN_BACK);
                return;
            }

            // iris/toggle/<eui>/<cap>
            if (topic.rfind("iris/toggle/", 0) == 0)
            {
                std::string rest = topic.substr(12);
                auto slash = rest.find('/');
                if (slash != std::string::npos)
                {
                    std::string eui_str = rest.substr(0, slash);
                    std::string cap_str = rest.substr(slash + 1);
                    uint8_t eui64[IRIS_EUI64_LEN];
                    uint8_t cap = 0;
                    if (cap_str == "inner")        cap = IRIS_CAP_INNER_LIGHT;
                    else if (cap_str == "outer")   cap = IRIS_CAP_OUTER_LIGHT;
                    else if (cap_str == "movement") cap = IRIS_CAP_MOVEMENT;
                    if (iris_str_to_eui64(eui_str.c_str(), eui64) && cap)
                        iris_toggle(eui64, cap);
                }
                return;
            }

            // iris/set_all/<cap>/on|off — explicit multicast state, not toggle
            if (topic == "iris/set_all/inner/on")  { iris_set_all(IRIS_CAP_INNER_LIGHT, true);  return; }
            if (topic == "iris/set_all/inner/off") { iris_set_all(IRIS_CAP_INNER_LIGHT, false); return; }
            if (topic == "iris/set_all/outer/on")  { iris_set_all(IRIS_CAP_OUTER_LIGHT, true);  return; }
            if (topic == "iris/set_all/outer/off") { iris_set_all(IRIS_CAP_OUTER_LIGHT, false); return; }
        });
}

#endif  // CONFIG_IRIS_ENABLED

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

#if defined(CONFIG_IRIS_ENABLED)
    if (iris_init() == ESP_OK)
    {
        iris_start_inventory_task();
        ESP_LOGI(TAG, "Iris Thread manager started (priority=%d)", iris_get_priority());
    }
    else
    {
        ESP_LOGE(TAG, "Iris Thread manager init failed");
    }
#endif

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

#if defined(CONFIG_IRIS_ENABLED)
            register_iris_providers();
            // Set initial master/backup status label
            Mercedes::getInstance().updateItemValue("master_status",
                iris_is_master() ? "" : "BACKUP");
            ESP_LOGI(TAG, "Iris dynamic screen providers registered");
#endif
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
