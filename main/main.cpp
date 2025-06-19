#include "app_task.h"
#include "bob.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void app_main(void)
    {
        bob_init();

        xTaskCreatePinnedToCore(app_task, "main_loop", 4096, NULL, tskIDLE_PRIORITY + 1, NULL, portNUM_PROCESSORS - 1);
    }
#ifdef __cplusplus
}
#endif
