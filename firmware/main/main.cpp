#include "app_task.h"
#include "ble_manager.h"
#include "esp_event.h"
#include "esp_insights.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "led_manager.h"
#include "led_status.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#define ESP_INSIGHTS_AUTH_KEY                                                                                          \
    "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9."                                                                            \
    "eyJ1c2VyIjoiZTYzNTNmOTUtN2I2Ni00M2U0LTgyM2UtOTlkYzAxNTYyN2NmIiwiaXNzIjoiZTMyMmI1OWMtNjNjYy00ZTQwLThlYTItNGU3NzY2" \
    "NTQ1Y2NhIiwic3ViIjoiMjE2YWJhNmYtZmI5Zi00ZTM3LWEzMDMtOTliZmNlODU1NWJiIiwiZXhwIjoyMDcxNDIzNjk0LCJpYXQiOjE3NTYwNjM2" \
    "OTR9.eG2musOILUiWUzE3AwWWx-_vOLeIoUlmL9LMaDrHYC6h_"                                                               \
    "YOYT4Fqtvytgv1qAI0jxXQmijoQdpoQrlNYQwJlH1gRpILcvlFdL1YkBjzfKXgo_"                                                 \
    "jJaOlmHv2tkd54FAg49DmG4j0BY3xAnhz5y0XBHsXWiFKwpZHWy0q5IuKyVJ3syNzmTg2LwVBVu8gU2EoGikdVKNazRC1BwPLz_"              \
    "KNWdW03WVCniun_"                                                                                                  \
    "2nVyZI5Y253Nch6MaeBpvrfRXhUI6uWXZuSDa3nrS5MmtElZgQjEyAJSX5lfhRwEc2Qi2LlHc4LHPD0YvO1JhSF4N6Rwf1FrJZ1qU_"           \
    "IxNTdTxtzLC0BUcYA"

#ifdef __cplusplus
extern "C"
{
#endif
    void app_main(void)
    {
        // Initialize NVS
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ESP_ERROR_CHECK(nvs_flash_init());
        }

        esp_insights_config_t config = {
            .log_type = ESP_DIAG_LOG_TYPE_ERROR,
            .node_id = nullptr,
            .auth_key = ESP_INSIGHTS_AUTH_KEY,
            .alloc_ext_ram = false,
        };

        esp_insights_init(&config);

        led_status_init(CONFIG_STATUS_WLED_PIN);

        // LED 0: solid red
        led_behavior_t led0_solid_red = {.mode = LED_MODE_SOLID, .color = {.r = 50, .g = 0, .b = 0}};
        led_status_set_behavior(0, led0_solid_red);

        // LED 1: fast blinking green (200ms an, 200ms aus)
        led_behavior_t led1_blink_green = {
            .mode = LED_MODE_BLINK, .color = {.r = 0, .g = 50, .b = 0}, .on_time_ms = 200, .off_time_ms = 200};
        led_status_set_behavior(1, led1_blink_green);

        // LED 2: slow blinking blue (1000ms an, 500ms aus)
        led_behavior_t led2_blink_blue = {
            .mode = LED_MODE_BLINK, .color = {.r = 0, .g = 0, .b = 50}, .on_time_ms = 1000, .off_time_ms = 500};
        led_status_set_behavior(2, led2_blink_blue);

        wled_init();
        register_handler();

        xTaskCreatePinnedToCore(app_task, "main_loop", 4096, NULL, tskIDLE_PRIORITY + 1, NULL, portNUM_PROCESSORS - 1);
        xTaskCreatePinnedToCore(ble_manager_task, "ble_manager", 4096, NULL, tskIDLE_PRIORITY + 1, NULL,
                                portNUM_PROCESSORS - 1);
    }
#ifdef __cplusplus
}
#endif
