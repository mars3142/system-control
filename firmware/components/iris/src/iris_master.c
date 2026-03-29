#include "iris/iris_internal.h"

#if defined(CONFIG_IRIS_ENABLED)

static const char *TAG = "Iris";

static void master_heartbeat_handler(void *ctx, otMessage *msg,
                                      const otMessageInfo *info)
{
    (void)ctx;
    if (!msg) return;

    char buf[64] = {};
    uint16_t len = otMessageGetLength(msg) - otMessageGetOffset(msg);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    otMessageRead(msg, otMessageGetOffset(msg), buf, len);

    cJSON *json = cJSON_Parse(buf);
    if (!json) return;
    cJSON *prio_item = cJSON_GetObjectItem(json, "priority");
    int peer_prio = cJSON_IsNumber(prio_item) ? (int)prio_item->valuedouble : 0;
    cJSON_Delete(json);

    int our_prio = (int)CONFIG_IRIS_MASTER_PRIORITY;

    if (peer_prio > our_prio && s_master_is_us) {
        // Higher-priority peer is alive → yield Master role
        ESP_LOGI(TAG, "Higher-priority master (prio=%d) detected — yielding", peer_prio);
        s_master_is_us = false;
        s_master_state = IRIS_MASTER_STANDBY;

        // Send yield acknowledgement
        char yield_buf[32];
        snprintf(yield_buf, sizeof(yield_buf), "{\"priority\":%d}", our_prio);

        otMessage *yield_msg = otCoapNewMessage(esp_openthread_get_instance(), NULL);
        if (yield_msg) {
            otCoapMessageInit(yield_msg, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
            otCoapMessageAppendUriPathOptions(yield_msg, "master_yield");
            otCoapMessageSetPayloadMarker(yield_msg);
            otMessageAppend(yield_msg, yield_buf, (uint16_t)strlen(yield_buf));
            otCoapSendRequest(esp_openthread_get_instance(), yield_msg, info, NULL, NULL);
        }

        otCommissionerStop(esp_openthread_get_instance());
    }
}

static void register_master_coap_resources(void)
{
    static otCoapResource s_res_heartbeat = {
        "master_heartbeat", master_heartbeat_handler, NULL, NULL
    };
    otCoapAddResource(esp_openthread_get_instance(), &s_res_heartbeat);
}

static void become_master(void)
{
    otInstance *inst = esp_openthread_get_instance();
    s_master_is_us = true;
    s_master_state = IRIS_MASTER_ACTIVE;

    otError err = otCommissionerStart(inst, NULL, joiner_callback, NULL);
    if (err != OT_ERROR_NONE) {
        ESP_LOGE(TAG, "Failed to start Commissioner: %d", err);
        s_master_is_us = false;
        s_master_state = IRIS_MASTER_STANDBY;
        return;
    }

    // Allow any joiner (wildcard) with our PSKd
    otCommissionerAddJoiner(inst, NULL, CONFIG_IRIS_JOINER_PSKD, 0xFFFFFFFF);
    ESP_LOGI(TAG, "Became Master (priority=%d)", CONFIG_IRIS_MASTER_PRIORITY);
}

void iris_master_task(void *arg)
{
    (void)arg;

    // Jitter 0–1000 ms to avoid simultaneous elections
    uint32_t jitter = (uint32_t)(esp_random() % 1000);
    vTaskDelay(pdMS_TO_TICKS(jitter));

    register_master_coap_resources();

    ESP_LOGI(TAG, "Starting election (priority=%d)", CONFIG_IRIS_MASTER_PRIORITY);
    become_master();

    TickType_t last_hb = xTaskGetTickCount();
    otInstance *inst   = esp_openthread_get_instance();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(CONFIG_IRIS_MASTER_HEARTBEAT_INTERVAL_MS));

        if (s_master_is_us) {
            // Send heartbeat to multicast group
            char hb_payload[32];
            snprintf(hb_payload, sizeof(hb_payload),
                     "{\"priority\":%d}", CONFIG_IRIS_MASTER_PRIORITY);

            otMessage *msg = otCoapNewMessage(inst, NULL);
            if (msg) {
                otCoapMessageInit(msg, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
                otCoapMessageAppendUriPathOptions(msg, "master_heartbeat");
                otCoapMessageSetPayloadMarker(msg);
                otMessageAppend(msg, hb_payload, (uint16_t)strlen(hb_payload));

                otMessageInfo info = {};
                otIp6AddressFromString("ff03::1", &info.mPeerAddr);
                info.mPeerPort = OT_DEFAULT_COAP_PORT;
                otCoapSendRequest(inst, msg, &info, NULL, NULL);
            }
            last_hb = xTaskGetTickCount();
        } else {
            // Check for heartbeat timeout → trigger failover
            TickType_t now    = xTaskGetTickCount();
            uint32_t elapsed  = (uint32_t)((now - last_hb) * portTICK_PERIOD_MS);
            if (elapsed >= CONFIG_IRIS_MASTER_FAILOVER_TIMEOUT_MS) {
                ESP_LOGW(TAG, "Master heartbeat timeout — starting election");
                s_master_state = IRIS_MASTER_INITIALIZING;
                become_master();
                last_hb = xTaskGetTickCount();
            }
        }
    }
}

#endif /* CONFIG_IRIS_ENABLED */
