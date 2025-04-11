#include "setup.h"
#include "freertos/idf_additions.h"

void app_task(void* param) {
    setup();
    while(1) {
        loop();
    }
}

#ifdef __cplusplus
extern "C" {
#endif
void app_main(void) {
    xTaskCreatePinnedToCore(app_task, "main_loop", 4096, NULL, 5, NULL, tskIDLE_PRIORITY + 1);
}
#ifdef __cplusplus
}
#endif
