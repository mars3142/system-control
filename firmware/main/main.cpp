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

static const char *TAG = "main";

extern const char insights_auth_key_start[] asm("_binary_insights_auth_key_txt_start");
extern const char insights_auth_key_end[] asm("_binary_insights_auth_key_txt_end");

void show_partition(void)
{
    const esp_partition_t *running_partition = esp_ota_get_running_partition();

    if (running_partition != NULL)
    {
        ESP_DIAG_EVENT(TAG, "Currently running partition: %s", running_partition->label);
        ESP_DIAG_EVENT(TAG, "  Type: %s", (running_partition->type == ESP_PARTITION_TYPE_APP) ? "APP" : "DATA");
        ESP_DIAG_EVENT(TAG, "  Subtype: %d", running_partition->subtype);
        ESP_DIAG_EVENT(TAG, "  Offset: 0x%lx", running_partition->address);
        ESP_DIAG_EVENT(TAG, "  Size: %ld bytes", running_partition->size);
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
            .auth_key = insights_auth_key_start,
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
