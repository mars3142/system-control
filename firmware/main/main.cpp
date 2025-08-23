#include "app_task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "led_manager.h"
#include "led_status.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void app_main(void)
    {
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

        xTaskCreatePinnedToCore(app_task, "main_loop", 4096, NULL, tskIDLE_PRIORITY + 1, NULL, portNUM_PROCESSORS - 1);

        wled_init();
        register_handler();
    }
#ifdef __cplusplus
}
#endif
