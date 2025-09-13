#include "app_task.h"
#include "ble_manager.h"
#include "esp_event.h"
#include "esp_insights.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_manager.h"
#include "led_status.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "wifi_manager.h"

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

        led_status_init(CONFIG_STATUS_WLED_PIN);

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
