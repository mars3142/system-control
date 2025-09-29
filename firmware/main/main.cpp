#include "app_task.h"
#include "color.h"
#include "hal_esp32/PersistenceManager.h"
#include "led_status.h"
#include "led_strip_ws2812.h"
#include "simulator.h"
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

    auto persistence = PersistenceManager();
    persistence.Load();

    led_status_init(CONFIG_STATUS_WLED_PIN);

    led_strip_init();
    start_simulation_task();

    xTaskCreatePinnedToCore(app_task, "app_task", 4096, NULL, tskIDLE_PRIORITY + 1, NULL, portNUM_PROCESSORS - 1);
    //  xTaskCreatePinnedToCore(ble_manager_task, "ble_manager", 4096, NULL, tskIDLE_PRIORITY + 1, NULL,
    //  portNUM_PROCESSORS - 1);

    if (persistence.GetValue("light_active", false))
    {
        led_strip_update(LED_STATE_DAY, rgb_t{});
    }
}
__END_DECLS
