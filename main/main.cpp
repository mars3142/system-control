#include "app_task.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "led_manager.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void app_main(void)
    {
        xTaskCreatePinnedToCore(app_task, "main_loop", 4096, NULL, tskIDLE_PRIORITY + 1, NULL, portNUM_PROCESSORS - 1);

        wled_init();
        register_handler();
    }
#ifdef __cplusplus
}
#endif
