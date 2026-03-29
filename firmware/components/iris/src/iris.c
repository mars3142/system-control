#include "iris/iris_internal.h"
#include "esp_openthread_lock.h"
#include <esp_random.h>
#include <openthread/dataset.h>

static const char *TAG = "Iris";

/* =========================================================================
 * Shared state — defined here, accessed by all sub-modules via iris_internal.h
 * ========================================================================= */

#if defined(CONFIG_IRIS_ENABLED)

iris_device_t        s_paired[CONFIG_IRIS_MAX_DEVICES];
int                  s_paired_count = 0;
SemaphoreHandle_t    s_mutex        = NULL;
TaskHandle_t         s_inventory_task_handle = NULL;
TaskHandle_t         s_master_task_handle    = NULL;

iris_master_state_t  s_master_state = IRIS_MASTER_INITIALIZING;
bool                 s_master_is_us = false;

iris_device_t        s_scan_cache[IRIS_SCAN_CACHE_MAX];
int                  s_scan_count     = 0;
volatile bool        s_discovery_active = false;

/* =========================================================================
 * Index lookup
 * ========================================================================= */

int find_device_index(const uint8_t eui64[IRIS_EUI64_LEN])
{
    for (int i = 0; i < s_paired_count; i++) {
        if (memcmp(s_paired[i].p.eui64, eui64, IRIS_EUI64_LEN) == 0)
            return i;
    }
    return -1;
}

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

static void iris_ot_main_task(void *arg)
{
    (void)arg;

    esp_openthread_lock_acquire(portMAX_DELAY);
    otInstance *instance = esp_openthread_get_instance();

    if (otDatasetIsCommissioned(instance) == false) {
        ESP_LOGW(TAG, "No commissioned dataset found, creating a new one.");
        otOperationalDataset dataset;
        memset(&dataset, 0, sizeof(dataset));

        // Set the channel
        dataset.mComponents.mIsChannelPresent = true;
        dataset.mChannel = 15;

        // Set the PAN ID
        dataset.mComponents.mIsPanIdPresent = true;
        dataset.mPanId = (otPanId)esp_random();

        // Set the Extended PAN ID
        dataset.mComponents.mIsExtendedPanIdPresent = true;
        esp_fill_random(dataset.mExtendedPanId.m8, sizeof(dataset.mExtendedPanId.m8));

        // Set the Network Name
        dataset.mComponents.mIsNetworkNamePresent = true;
        snprintf((char *)dataset.mNetworkName.m8, sizeof(dataset.mNetworkName.m8), "sys-ctrl-%04x",
                 (uint16_t)esp_random());
        
        // Set the Network Key
        dataset.mComponents.mIsNetworkKeyPresent = true;
        esp_fill_random(dataset.mNetworkKey.m8, sizeof(dataset.mNetworkKey.m8));

        ESP_ERROR_CHECK(otDatasetSetActive(instance, &dataset));
    }

    otCoapStart(instance, OT_DEFAULT_COAP_PORT);
    otThreadRegisterNeighborTableCallback(instance, iris_neighbor_callback);
    
    // Start the network
    otIp6SetEnabled(instance, true);
    otThreadSetEnabled(instance, true);

    esp_openthread_lock_release();

    // esp_openthread_launch_mainloop() blocks until OpenThread is deinitialized
    esp_openthread_launch_mainloop();
    vTaskDelete(NULL);
}

esp_err_t iris_init(void)
{
    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) return ESP_ERR_NO_MEM;

    spiffs_load();

    esp_vfs_eventfd_config_t eventfd_config = { .max_fds = 3 };
    esp_vfs_eventfd_register(&eventfd_config);

    esp_openthread_platform_config_t ot_config = {
        .radio_config = {
            .radio_mode = RADIO_MODE_NATIVE,
        },
        .host_config = {
            .host_connection_mode = HOST_CONNECTION_MODE_NONE,
        },
        .port_config = {
            .storage_partition_name = "nvs",
            .netif_queue_size       = 10,
            .task_queue_size        = 10,
        },
    };

    esp_err_t err = esp_openthread_init(&ot_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "OpenThread init failed: %s", esp_err_to_name(err));
        return err;
    }

    // Launch OpenThread mainloop in a dedicated task (required by ESP-IDF)
    xTaskCreate(iris_ot_main_task, "ot_main", 8192, NULL,
                tskIDLE_PRIORITY + 4, NULL);

    ESP_LOGI(TAG, "Iris initialised — %d device(s) loaded", s_paired_count);
    return ESP_OK;
}

void iris_start_inventory_task(void)
{
    xTaskCreate(iris_inventory_task, "iris_inv", 4096, NULL,
                tskIDLE_PRIORITY + 2, &s_inventory_task_handle);

    xTaskCreate(iris_master_task, "iris_master", 4096, NULL,
                tskIDLE_PRIORITY + 3, &s_master_task_handle);
}

/* =========================================================================
 * Device discovery and provisioning
 * ========================================================================= */

int iris_scan(iris_device_t *out, int max)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    int n = s_scan_count < max ? s_scan_count : max;
    memcpy(out, s_scan_cache, n * sizeof(iris_device_t));
    xSemaphoreGive(s_mutex);
    return n;
}

esp_err_t iris_pair(const uint8_t eui64[IRIS_EUI64_LEN], const char *name)
{
    if (!s_master_is_us) {
        ESP_LOGW(TAG, "iris_pair called on Backup — ignoring");
        return ESP_ERR_NOT_SUPPORTED;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    if (s_paired_count >= CONFIG_IRIS_MAX_DEVICES) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NO_MEM;
    }
    if (find_device_index(eui64) >= 0) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_STATE;  // already paired
    }
    xSemaphoreGive(s_mutex);

    // Query capabilities via CoAP
    otIp6Address addr;
    uint8_t caps = 0;
    if (eui64_to_ml_eid(eui64, &addr)) {
        char resp[64] = {};
        if (coap_get(&addr, "capabilities", resp, sizeof(resp))) {
            cJSON *json = cJSON_Parse(resp);
            if (json) {
                cJSON *c = cJSON_GetObjectItem(json, "caps");
                if (cJSON_IsNumber(c)) caps = (uint8_t)c->valuedouble;
                cJSON_Delete(json);
            }
        }
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    iris_device_t *dev = &s_paired[s_paired_count];
    memset(dev, 0, sizeof(*dev));
    memcpy(dev->p.eui64, eui64, IRIS_EUI64_LEN);
    strncpy(dev->p.name, name ? name : "Unknown", IRIS_DEVICE_NAME_MAX - 1);
    dev->p.capabilities = caps;
    dev->online         = true;
    s_paired_count++;

    // Remove from scan cache
    for (int i = 0; i < s_scan_count; i++) {
        if (memcmp(s_scan_cache[i].p.eui64, eui64, IRIS_EUI64_LEN) == 0) {
            s_scan_cache[i] = s_scan_cache[--s_scan_count];
            break;
        }
    }

    spiffs_save();
    xSemaphoreGive(s_mutex);

    char eui_str[17];
    iris_eui64_to_str(eui64, eui_str, sizeof(eui_str));
    ESP_LOGI(TAG, "Paired device %s ('%s') caps=0x%02x", eui_str, name, caps);
    return ESP_OK;
}

/* =========================================================================
 * Paired device management
 * ========================================================================= */

int iris_get_paired(iris_device_t *out, int max)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    int n = s_paired_count < max ? s_paired_count : max;
    memcpy(out, s_paired, n * sizeof(iris_device_t));
    xSemaphoreGive(s_mutex);
    return n;
}

esp_err_t iris_toggle(const uint8_t eui64[IRIS_EUI64_LEN], uint8_t cap)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    int idx = find_device_index(eui64);
    bool online = (idx >= 0) ? s_paired[idx].online : false;
    xSemaphoreGive(s_mutex);

    if (idx < 0) return ESP_ERR_NOT_FOUND;
    if (!online) {
        ESP_LOGD(TAG, "iris_toggle: device offline — skipping");
        return ESP_OK;
    }

    otIp6Address addr;
    if (!eui64_to_ml_eid(eui64, &addr)) return ESP_FAIL;

    char payload[32];
    snprintf(payload, sizeof(payload), "{\"cap\":%u}", (unsigned)cap);
    coap_post(&addr, "toggle", payload);
    return ESP_OK;
}

esp_err_t iris_set_all(uint8_t cap, bool on)
{
    otInstance *inst = esp_openthread_get_instance();
    if (!inst) return ESP_FAIL;

    char payload[48];
    snprintf(payload, sizeof(payload), "{\"cap\":%u,\"state\":%u}",
             (unsigned)cap, on ? 1u : 0u);

    otMessage *msg = otCoapNewMessage(inst, NULL);
    if (!msg) return ESP_ERR_NO_MEM;

    otCoapMessageInit(msg, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_POST);
    otCoapMessageAppendUriPathOptions(msg, "set");
    otCoapMessageAppendContentFormatOption(msg, OT_COAP_OPTION_CONTENT_FORMAT_JSON);
    otCoapMessageSetPayloadMarker(msg);
    otMessageAppend(msg, payload, (uint16_t)strlen(payload));

    otMessageInfo info = {};
    otIp6AddressFromString("ff03::1", &info.mPeerAddr);
    info.mPeerPort = OT_DEFAULT_COAP_PORT;

    otError err = otCoapSendRequest(inst, msg, &info, NULL, NULL);
    ESP_LOGI(TAG, "Multicast set cap=0x%02x state=%d: %s",
             cap, (int)on, err == OT_ERROR_NONE ? "ok" : "fail");
    return (err == OT_ERROR_NONE) ? ESP_OK : ESP_FAIL;
}

esp_err_t iris_rename(const uint8_t eui64[IRIS_EUI64_LEN], const char *new_name)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    int idx = find_device_index(eui64);
    if (idx < 0) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }
    strncpy(s_paired[idx].p.name, new_name, IRIS_DEVICE_NAME_MAX - 1);
    s_paired[idx].p.name[IRIS_DEVICE_NAME_MAX - 1] = '\0';
    spiffs_save();
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t iris_unpair(const uint8_t eui64[IRIS_EUI64_LEN])
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    int idx = find_device_index(eui64);
    if (idx < 0) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }
    for (int i = idx; i < s_paired_count - 1; i++)
        s_paired[i] = s_paired[i + 1];
    s_paired_count--;
    spiffs_save();
    xSemaphoreGive(s_mutex);

    char eui_str[17];
    iris_eui64_to_str(eui64, eui_str, sizeof(eui_str));
    ESP_LOGI(TAG, "Unpaired device %s", eui_str);
    return ESP_OK;
}

/* =========================================================================
 * Capability query helpers
 * ========================================================================= */

bool iris_any_has_cap(uint8_t cap)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool found = false;
    for (int i = 0; i < s_paired_count && !found; i++)
        if (s_paired[i].p.capabilities & cap) found = true;
    xSemaphoreGive(s_mutex);
    return found;
}

/* =========================================================================
 * Master / Backup state
 * ========================================================================= */

bool iris_is_master(void)
{
    return s_master_is_us;
}

uint8_t iris_get_priority(void)
{
    return (uint8_t)CONFIG_IRIS_MASTER_PRIORITY;
}

/* =========================================================================
 * Stub implementations when IRIS is disabled
 * ========================================================================= */

#else  /* CONFIG_IRIS_ENABLED not set */

esp_err_t iris_init(void) { return ESP_ERR_NOT_SUPPORTED; }
void      iris_start_inventory_task(void) {}
void      iris_run_discovery(void) {}
int       iris_scan(iris_device_t *out, int max) { (void)out; (void)max; return 0; }
esp_err_t iris_pair(const uint8_t eui64[IRIS_EUI64_LEN], const char *name) { (void)eui64; (void)name; return ESP_ERR_NOT_SUPPORTED; }
int       iris_get_paired(iris_device_t *out, int max) { (void)out; (void)max; return 0; }
esp_err_t iris_toggle(const uint8_t eui64[IRIS_EUI64_LEN], uint8_t cap) { (void)eui64; (void)cap; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t iris_set_all(uint8_t cap, bool on) { (void)cap; (void)on; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t iris_rename(const uint8_t eui64[IRIS_EUI64_LEN], const char *new_name) { (void)eui64; (void)new_name; return ESP_ERR_NOT_SUPPORTED; }
esp_err_t iris_unpair(const uint8_t eui64[IRIS_EUI64_LEN]) { (void)eui64; return ESP_ERR_NOT_SUPPORTED; }
bool      iris_any_has_cap(uint8_t cap) { (void)cap; return false; }
bool      iris_is_master(void) { return false; }
uint8_t   iris_get_priority(void) { return 0; }

#endif  /* CONFIG_IRIS_ENABLED */

/* =========================================================================
 * EUI-64 utility (always compiled — used by both enabled and stub paths)
 * ========================================================================= */

void iris_eui64_to_str(const uint8_t eui64[IRIS_EUI64_LEN], char *out, size_t len)
{
    if (len < 17) return;
    snprintf(out, len,
             "%02x%02x%02x%02x%02x%02x%02x%02x",
             eui64[0], eui64[1], eui64[2], eui64[3],
             eui64[4], eui64[5], eui64[6], eui64[7]);
}

bool iris_str_to_eui64(const char *str, uint8_t eui64[IRIS_EUI64_LEN])
{
    if (!str || strlen(str) != 16) return false;
    for (int i = 0; i < 8; i++) {
        char byte_str[3] = { str[i * 2], str[i * 2 + 1], '\0' };
        if (!isxdigit((unsigned char)byte_str[0]) || !isxdigit((unsigned char)byte_str[1]))
            return false;
        eui64[i] = (uint8_t)strtoul(byte_str, NULL, 16);
    }
    return true;
}
