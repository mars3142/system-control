#include "iris/iris_internal.h"

#if defined(CONFIG_IRIS_ENABLED)

static const char *TAG = "Iris";

void iris_inventory_task(void *arg)
{
    (void)arg;

    // Run an initial discovery sweep on startup so already-joined devices
    // are found immediately (e.g. after a firmware flash that wiped SPIFFS).
    iris_run_discovery();

    int cycles_since_discovery = 0;

    while (true) {
        // Wait for the poll interval OR an early wake-up from the neighbor
        // callback (when a known device rejoins the network).
        xTaskNotifyWait(0, 0, NULL,
                        pdMS_TO_TICKS(CONFIG_IRIS_INVENTORY_INTERVAL_MS));

        // Periodic re-discovery: runs every IRIS_DISCOVERY_INTERVAL_CYCLES
        // poll cycles to catch devices that joined while C6 was not listening.
        cycles_since_discovery++;
        if (cycles_since_discovery >= CONFIG_IRIS_DISCOVERY_INTERVAL_CYCLES) {
            iris_run_discovery();
            cycles_since_discovery = 0;
        }

        xSemaphoreTake(s_mutex, portMAX_DELAY);
        int count = s_paired_count;
        xSemaphoreGive(s_mutex);

        for (int i = 0; i < count; i++) {
            uint8_t eui64[IRIS_EUI64_LEN];

            xSemaphoreTake(s_mutex, portMAX_DELAY);
            if (i >= s_paired_count) {
                xSemaphoreGive(s_mutex);
                break;
            }
            memcpy(eui64, s_paired[i].p.eui64, IRIS_EUI64_LEN);
            xSemaphoreGive(s_mutex);

            otIp6Address addr;
            if (!eui64_to_ml_eid(eui64, &addr)) continue;

            char resp_buf[64] = {};
            bool ok = coap_get(&addr, "state", resp_buf, sizeof(resp_buf));

            xSemaphoreTake(s_mutex, portMAX_DELAY);
            if (i >= s_paired_count ||
                memcmp(s_paired[i].p.eui64, eui64, IRIS_EUI64_LEN) != 0) {
                xSemaphoreGive(s_mutex);
                continue;
            }

            if (ok) {
                cJSON *json = cJSON_Parse(resp_buf);
                if (json) {
                    cJSON *st = cJSON_GetObjectItem(json, "state");
                    if (cJSON_IsNumber(st))
                        s_paired[i].p.state = (uint8_t)st->valuedouble;
                    cJSON_Delete(json);
                }
                s_paired[i].online       = true;
                s_paired[i].failed_polls = 0;
            } else {
                s_paired[i].failed_polls++;
                if (s_paired[i].failed_polls >= CONFIG_IRIS_OFFLINE_THRESHOLD) {
                    if (s_paired[i].online) {
                        char eui_str[17];
                        iris_eui64_to_str(eui64, eui_str, sizeof(eui_str));
                        ESP_LOGW(TAG, "Device %s went offline", eui_str);
                    }
                    s_paired[i].online = false;
                }
            }
            xSemaphoreGive(s_mutex);
        }
    }
}

#endif /* CONFIG_IRIS_ENABLED */
