#include "iris/iris_internal.h"
#include "esp_openthread_lock.h"

#if defined(CONFIG_IRIS_ENABLED)

static const char *TAG = "Iris";

/**
 * @brief Called by OpenThread when a Thread neighbor is added or removed.
 *
 * When a paired device rejoins the network after being offline, this callback
 * fires immediately — much faster than waiting for the 30-second inventory
 * poll. We mark the device online and reset the failed_polls counter so the
 * OLED menu shows it as available right away.
 *
 * Registered in iris_init(). Runs in the OpenThread task context.
 */
void iris_neighbor_callback(otNeighborTableEvent event,
                            const otNeighborTableEntryInfo *info)
{
    if (event != OT_NEIGHBOR_TABLE_EVENT_CHILD_ADDED &&
        event != OT_NEIGHBOR_TABLE_EVENT_ROUTER_ADDED)
        return;

    const uint8_t *ext = (event == OT_NEIGHBOR_TABLE_EVENT_CHILD_ADDED)
                             ? info->mInfo.mChild.mExtAddress.m8
                             : info->mInfo.mRouter.mExtAddress.m8;

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    int idx = find_device_index(ext);
    bool newly_online = (idx >= 0 && !s_paired[idx].online);
    if (newly_online) {
        s_paired[idx].online       = true;
        s_paired[idx].failed_polls = 0;
        char eui_str[17];
        iris_eui64_to_str(ext, eui_str, sizeof(eui_str));
        ESP_LOGI(TAG, "Auto-discovery: paired device %s back online", eui_str);
    }
    xSemaphoreGive(s_mutex);

    // Wake the inventory task immediately to fetch current state without
    // waiting for the next 30 s poll cycle.
    if (newly_online && s_inventory_task_handle)
        xTaskNotify(s_inventory_task_handle, 0, eNoAction);
}

/**
 * @brief Commissioner joiner callback — fires when a new H2 calls otJoinerStart().
 *
 * Registered in become_master() (iris_master.c). Runs in the OpenThread task context.
 */
void joiner_callback(otCommissionerJoinerEvent event,
                     const otJoinerInfo *info,
                     const otExtAddress *eui64,
                     void *ctx)
{
    (void)info;
    (void)ctx;
    if (event != OT_COMMISSIONER_JOINER_CONNECTED)
        return;

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (s_scan_count < IRIS_SCAN_CACHE_MAX) {
        iris_device_t dev = {};
        memcpy(dev.p.eui64, eui64->m8, IRIS_EUI64_LEN);

        char eui_str[17];
        iris_eui64_to_str(dev.p.eui64, eui_str, sizeof(eui_str));
        snprintf(dev.p.name, IRIS_DEVICE_NAME_MAX, "H2-%s", eui_str + 8);
        dev.online = true;

        bool found = false;
        for (int i = 0; i < s_scan_count; i++) {
            if (memcmp(s_scan_cache[i].p.eui64, dev.p.eui64, IRIS_EUI64_LEN) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            s_scan_cache[s_scan_count++] = dev;
            ESP_LOGI(TAG, "New joiner discovered: %s", eui_str);
        }
    }
    xSemaphoreGive(s_mutex);
}

/**
 * @brief CoAP response handler for multicast GET /discover.
 *
 * Called once per responding H2 device. Distinguishes three device states:
 *
 *  1. PAIRED   — EUI-64 is in iris_devices.bin.
 *                → mark online, update state.
 *
 *  2. REJOINED — EUI-64 is NOT in iris_devices.bin, but the device is already
 *                in the Thread network (SPIFFS record was lost, e.g. after flash).
 *                → auto-restore to iris_devices.bin immediately.
 *
 *  3. NEW JOINER — Never provisioned. Appears via joiner_callback, NOT /discover.
 *                  Goes to s_scan_cache[]; requires manual "Aufnehmen".
 *
 * Runs in the OpenThread task context.
 */
static void discover_response_handler(void *ctx, otMessage *msg,
                                       const otMessageInfo *info, otError err)
{
    (void)ctx;
    (void)info;
    if (!s_discovery_active || err != OT_ERROR_NONE || !msg) return;

    char buf[96] = {};
    uint16_t len = otMessageGetLength(msg) - otMessageGetOffset(msg);
    if (len >= sizeof(buf)) len = (uint16_t)(sizeof(buf) - 1);
    otMessageRead(msg, otMessageGetOffset(msg), buf, len);

    cJSON *json = cJSON_Parse(buf);
    if (!json) return;

    cJSON *eui_item  = cJSON_GetObjectItem(json, "eui64");
    cJSON *caps_item = cJSON_GetObjectItem(json, "caps");
    cJSON *st_item   = cJSON_GetObjectItem(json, "state");
    cJSON *name_item = cJSON_GetObjectItem(json, "name");

    if (!cJSON_IsString(eui_item) || !cJSON_IsNumber(caps_item)) {
        cJSON_Delete(json);
        return;
    }

    uint8_t eui64[IRIS_EUI64_LEN];
    if (!iris_str_to_eui64(eui_item->valuestring, eui64)) {
        cJSON_Delete(json);
        return;
    }

    uint8_t caps  = (uint8_t)caps_item->valuedouble;
    uint8_t state = cJSON_IsNumber(st_item) ? (uint8_t)st_item->valuedouble : 0;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    int idx = find_device_index(eui64);
    if (idx >= 0) {
        // PAIRED — update runtime fields
        s_paired[idx].online       = true;
        s_paired[idx].failed_polls = 0;
        s_paired[idx].p.state      = state;
        ESP_LOGD(TAG, "Discovery: paired device %s online", eui_item->valuestring);
    } else if (s_paired_count < CONFIG_IRIS_MAX_DEVICES) {
        // REJOINED — auto-restore
        iris_device_t dev = {};
        memcpy(dev.p.eui64, eui64, IRIS_EUI64_LEN);
        if (cJSON_IsString(name_item) && name_item->valuestring[0])
            strncpy(dev.p.name, name_item->valuestring, IRIS_DEVICE_NAME_MAX - 1);
        else
            snprintf(dev.p.name, IRIS_DEVICE_NAME_MAX, "H2-%s",
                     eui_item->valuestring + 8);
        dev.p.capabilities = caps;
        dev.p.state        = state;
        dev.online         = true;
        s_paired[s_paired_count++] = dev;
        spiffs_save();
        ESP_LOGI(TAG, "Discovery: rejoined device %s auto-restored (caps=0x%02x)",
                 eui_item->valuestring, caps);
    } else {
        ESP_LOGW(TAG, "Discovery: rejoined device %s found but paired list full",
                 eui_item->valuestring);
    }

    xSemaphoreGive(s_mutex);
    cJSON_Delete(json);
}

void iris_run_discovery(void)
{
    esp_openthread_lock_acquire(portMAX_DELAY);

    otInstance *inst = esp_openthread_get_instance();
    if (!inst) {
        esp_openthread_lock_release();
        return;
    }

    ESP_LOGI(TAG, "Starting discovery sweep...");

    otMessage *msg = otCoapNewMessage(inst, NULL);
    if (!msg) {
        esp_openthread_lock_release();
        return;
    }

    otCoapMessageInit(msg, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapMessageGenerateToken(msg, OT_COAP_DEFAULT_TOKEN_LENGTH);
    if (otCoapMessageAppendUriPathOptions(msg, "discover") != OT_ERROR_NONE) {
        otMessageFree(msg);
        esp_openthread_lock_release();
        return;
    }

    otMessageInfo info = {};
    otIp6AddressFromString("ff03::1", &info.mPeerAddr);
    info.mPeerPort = OT_DEFAULT_COAP_PORT;

    s_discovery_active = true;
    otError err = otCoapSendRequest(inst, msg, &info,
                                    discover_response_handler, NULL);

    esp_openthread_lock_release();

    if (err != OT_ERROR_NONE) {
        s_discovery_active = false;
        ESP_LOGW(TAG, "Discovery send failed: %d", err);
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(CONFIG_IRIS_DISCOVERY_WINDOW_MS));
    s_discovery_active = false;

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    ESP_LOGI(TAG, "Discovery complete — %d paired, %d new in cache",
             s_paired_count, s_scan_count);
    xSemaphoreGive(s_mutex);
}

#endif /* CONFIG_IRIS_ENABLED */
