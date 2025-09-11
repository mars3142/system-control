#include "app_task.h"
#include "ble_manager.h"
#include "esp_event.h"
#include "esp_insights.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_rmaker_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_manager.h"
#include "led_status.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "wifi_handler.h"

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

static const char *TAG = "main";

void show_partition(void)
{
    const esp_partition_t *running_partition = esp_ota_get_running_partition();

    if (running_partition != NULL)
    {
        ESP_LOGI(TAG, "Currently running partition: %s", running_partition->label);
        ESP_LOGI(TAG, "  Type: %s", (running_partition->type == ESP_PARTITION_TYPE_APP) ? "APP" : "DATA");
        ESP_LOGI(TAG, "  Subtype: %d", running_partition->subtype);
        ESP_LOGI(TAG, "  Offset: 0x%lx", running_partition->address);
        ESP_LOGI(TAG, "  Size: %ld bytes", running_partition->size);
    }
    else
    {
        ESP_LOGE(TAG, "Could not determine the running partition!");
    }
}

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

        show_partition();

        led_status_init(CONFIG_STATUS_WLED_PIN);

        wifi_init_sta();

        esp_insights_config_t config = {
            .log_type = ESP_DIAG_LOG_TYPE_ERROR,
            .node_id = nullptr,
            .auth_key = ESP_INSIGHTS_AUTH_KEY,
            .alloc_ext_ram = false,
        };

        esp_insights_init(&config);

        esp_rmaker_time_sync_init(NULL);

        wled_init();

        register_handler();

        xTaskCreatePinnedToCore(app_task, "main_loop", 4096, NULL, tskIDLE_PRIORITY + 1, NULL, portNUM_PROCESSORS - 1);
        // xTaskCreatePinnedToCore(ble_manager_task, "ble_manager", 4096, NULL, tskIDLE_PRIORITY + 1, NULL,
        // portNUM_PROCESSORS - 1);

        led_event_data_t payload = {.value = 42};
        send_event(EVENT_LED_ON, &payload);
    }
#ifdef __cplusplus
}
#endif
