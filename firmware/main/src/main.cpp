#include "app_task.h"
#include "color.h"
#include "led_status.h"
#include "led_strip_ws2812.h"
#include "persistence_manager.h"
#include "wifi_manager.h"
#include <ble_manager.h>
#include <esp_event.h>
#include <esp_insights.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <sdkconfig.h>

__BEGIN_DECLS
void app_main(void)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    persistence_manager_t persistence;
    persistence_manager_init(&persistence, "config");
    persistence_manager_load(&persistence);

    led_status_init(CONFIG_STATUS_WLED_PIN);

    led_strip_init();

    xTaskCreatePinnedToCore(app_task, "app_task", 8192, NULL, tskIDLE_PRIORITY + 5, NULL,
                            CONFIG_FREERTOS_NUMBER_OF_CORES - 1);
    // xTaskCreatePinnedToCore(ble_manager_task, "ble_manager", 4096, NULL, tskIDLE_PRIORITY + 1, NULL,
    //                         CONFIG_FREERTOS_NUMBER_OF_CORES - 1);
}
__END_DECLS
