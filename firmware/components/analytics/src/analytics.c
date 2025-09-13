#include "analytics.h"
#include "esp_insights.h"
#include "esp_rmaker_utils.h"

extern const char insights_auth_key_start[] asm("_binary_insights_auth_key_txt_start");
extern const char insights_auth_key_end[] asm("_binary_insights_auth_key_txt_end");

void analytics_init(void)
{
    esp_insights_config_t config = {
        .log_type = ESP_DIAG_LOG_TYPE_ERROR | ESP_DIAG_LOG_TYPE_EVENT | ESP_DIAG_LOG_TYPE_WARNING,
        .node_id = NULL,
        .auth_key = insights_auth_key_start,
        .alloc_ext_ram = false,
    };

    esp_insights_init(&config);

    esp_rmaker_time_sync_init(NULL);
}
