#include "thread_manager.h"
#include "esp_ot_config.h"
#include "led_status.h"

#include <stdio.h>
#include <string.h>

#include "esp_check.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_openthread.h"
#include "esp_openthread_lock.h"
#include "esp_openthread_netif_glue.h"
#include "esp_openthread_types.h"
#include "esp_vfs_eventfd.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "openthread/coap.h"
#include "openthread/commissioner.h"
#include "openthread/dataset.h"
#include "openthread/dataset_ftd.h"
#include "openthread/instance.h"
#include "openthread/ip6.h"
#include "openthread/link.h"
#include "openthread/thread.h"
#include "openthread/thread_ftd.h"
#include "sdkconfig.h"

static const char *TAG = "thread_manager";

#define COMMISSIONER_PSK CONFIG_THREAD_JOINER_PSK
#define NVS_NAMESPACE "thread_mgr"
#define THREAD_LED_INDEX 0

static thread_event_cb_t s_event_cb;
static SemaphoreHandle_t s_mutex;
static thread_device_t s_devices[THREAD_MAX_DEVICES];
static size_t s_device_count;
static thread_group_t s_groups[THREAD_MAX_GROUPS];
static size_t s_group_count;
static otCoapResource s_announce_resource;
static bool s_coap_started;
static TimerHandle_t s_become_leader_timer;
static TimerHandle_t s_add_joiner_timer;

// ─── Helpers ─────────────────────────────────────────────────────────────────

static void emit(const char *json)
{
    if (s_event_cb)
        s_event_cb(json);
}

static thread_device_t *find_device_by_addr(const char *addr)
{
    for (size_t i = 0; i < s_device_count; ++i)
    {
        if (strcmp(s_devices[i].ipv6_addr, addr) == 0)
            return &s_devices[i];
    }
    return NULL;
}

static thread_group_t *find_group_by_addr(const char *addr)
{
    for (size_t i = 0; i < s_group_count; ++i)
    {
        if (strcmp(s_groups[i].multicast_addr, addr) == 0)
            return &s_groups[i];
    }
    return NULL;
}

// ─── NVS Persistence ─────────────────────────────────────────────────────────

static void persist_devices(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
        return;
    if (s_device_count > 0)
        nvs_set_blob(h, "devices", s_devices, s_device_count * sizeof(thread_device_t));
    else
        nvs_erase_key(h, "devices");
    nvs_commit(h);
    nvs_close(h);
}

static void load_devices(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK)
        return;
    size_t size = sizeof(s_devices);
    if (nvs_get_blob(h, "devices", s_devices, &size) == ESP_OK)
    {
        s_device_count = size / sizeof(thread_device_t);
        for (size_t i = 0; i < s_device_count; ++i)
            s_devices[i].reachable = false;
    }
    nvs_close(h);
    ESP_LOGI(TAG, "Loaded %zu device(s) from NVS", s_device_count);
}

static void persist_groups(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
        return;
    if (s_group_count > 0)
        nvs_set_blob(h, "groups", s_groups, s_group_count * sizeof(thread_group_t));
    else
        nvs_erase_key(h, "groups");
    nvs_commit(h);
    nvs_close(h);
}

static void load_groups(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK)
        return;
    size_t size = sizeof(s_groups);
    if (nvs_get_blob(h, "groups", s_groups, &size) == ESP_OK)
        s_group_count = size / sizeof(thread_group_t);
    nvs_close(h);
    ESP_LOGI(TAG, "Loaded %zu group(s) from NVS", s_group_count);
}

// ─── LED helpers ─────────────────────────────────────────────────────────────

static void update_thread_led(void)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    bool has_devices = s_device_count > 0;
    bool any_reachable = false;
    bool any_unreachable = false;
    for (size_t i = 0; i < s_device_count; ++i)
    {
        if (s_devices[i].reachable)
            any_reachable = true;
        else
            any_unreachable = true;
    }
    xSemaphoreGive(s_mutex);

    led_behavior_t b = {.index = THREAD_LED_INDEX};
    if (!has_devices)
    {
        b.mode = LED_MODE_OFF;
    }
    else if (any_reachable && !any_unreachable)
    {
        b.mode = LED_MODE_SOLID;
        b.color = (rgb_t){.red = 0, .green = 0, .blue = 64};
    }
    else if (any_reachable && any_unreachable)
    {
        b.mode = LED_MODE_BLINK_ALT;
        b.color = (rgb_t){.red = 0, .green = 0, .blue = 64};
        b.alt_color = (rgb_t){.red = 64, .green = 0, .blue = 0};
        b.on_time_ms = 500;
        b.off_time_ms = 500;
    }
    else
    {
        b.mode = LED_MODE_SOLID;
        b.color = (rgb_t){.red = 64, .green = 0, .blue = 0};
    }
    led_status_set_behavior(b);
}

// ─── CoAP Observe (RFC 7641) ─────────────────────────────────────────────────

typedef struct
{
    char addr[46];
    char resource[16];
} observe_ctx_t;

static void send_observe_cancel(otInstance *instance, const otMessage *notif, const otMessageInfo *msg_info,
                                const char *resource)
{
    otMessage *cancel = otCoapNewMessage(instance, NULL);
    if (!cancel)
        return;

    otCoapMessageInit(cancel, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapMessageSetToken(cancel, otCoapMessageGetToken(notif), otCoapMessageGetTokenLength(notif));
    otCoapMessageAppendObserveOption(cancel, 1); // 1 = deregister
    otCoapMessageAppendUriPathOptions(cancel, resource);

    if (otCoapSendRequest(instance, cancel, msg_info, NULL, NULL) != OT_ERROR_NONE)
        otMessageFree(cancel);
}

static void observe_notification_cb(void *context, otMessage *message, const otMessageInfo *msg_info, otError result)
{
    observe_ctx_t *ctx = (observe_ctx_t *)context;

    if (result != OT_ERROR_NONE || !message)
    {
        ESP_LOGI(TAG, "Observe ended %s/%s", ctx->addr, ctx->resource);

        bool was_reachable = false;
        xSemaphoreTake(s_mutex, portMAX_DELAY);
        thread_device_t *dev = find_device_by_addr(ctx->addr);
        if (dev && dev->reachable)
        {
            dev->reachable = false;
            was_reachable = true;
        }
        xSemaphoreGive(s_mutex);

        if (was_reachable)
        {
            update_thread_led();
            char json[128];
            snprintf(json, sizeof(json), "{\"type\":\"thread_unreachable\",\"addr\":\"%s\"}", ctx->addr);
            emit(json);
        }

        free(ctx);
        return;
    }

    if (otCoapMessageGetCode(message) != OT_COAP_CODE_CONTENT)
    {
        free(ctx);
        return;
    }

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    thread_device_t *dev = find_device_by_addr(ctx->addr);
    xSemaphoreGive(s_mutex);

    if (!dev)
    {
        // Device was removed — send deregister so H2 frees the observer slot
        send_observe_cancel(esp_openthread_get_instance(), message, msg_info, ctx->resource);
        free(ctx);
        return;
    }

    char payload[4] = {};
    uint16_t len = otMessageRead(message, otMessageGetOffset(message), payload, sizeof(payload) - 1);
    payload[len] = '\0';
    bool is_on = (payload[0] == '1');

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    dev = find_device_by_addr(ctx->addr);
    if (dev)
    {
        dev->reachable = true;
        if (strcmp(ctx->resource, "beacon") == 0)
            dev->beacon_on = is_on;
        else if (strcmp(ctx->resource, "outdoor") == 0)
            dev->outdoor_on = is_on;
    }
    xSemaphoreGive(s_mutex);

    char json[256];
    snprintf(json, sizeof(json), "{\"type\":\"thread_state\",\"addr\":\"%s\",\"resource\":\"%s\",\"on\":%s}", ctx->addr,
             ctx->resource, is_on ? "true" : "false");
    emit(json);

    // ctx stays alive — callback fires again for each subsequent notification
}

static void register_observe(otInstance *instance, const char *addr_str, const char *resource)
{
    otMessage *msg = otCoapNewMessage(instance, NULL);
    if (!msg)
        return;

    otCoapMessageInit(msg, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapMessageGenerateToken(msg, OT_COAP_DEFAULT_TOKEN_LENGTH);
    otCoapMessageAppendObserveOption(msg, 0); // 0 = register

    if (otCoapMessageAppendUriPathOptions(msg, resource) != OT_ERROR_NONE)
    {
        otMessageFree(msg);
        return;
    }

    otMessageInfo info;
    memset(&info, 0, sizeof(info));
    otIp6AddressFromString(addr_str, &info.mPeerAddr);
    info.mPeerPort = OT_DEFAULT_COAP_PORT;

    observe_ctx_t *ctx = malloc(sizeof(observe_ctx_t));
    if (!ctx)
    {
        otMessageFree(msg);
        return;
    }
    strlcpy(ctx->addr, addr_str, sizeof(ctx->addr));
    strlcpy(ctx->resource, resource, sizeof(ctx->resource));

    if (otCoapSendRequest(instance, msg, &info, observe_notification_cb, ctx) != OT_ERROR_NONE)
    {
        otMessageFree(msg);
        free(ctx);
        return;
    }
    ESP_LOGI(TAG, "Observe registered: %s/%s", addr_str, resource);
}

static void register_all_observers(otInstance *instance)
{
    // Snapshot under mutex, then register outside
    thread_device_t snap[THREAD_MAX_DEVICES];
    size_t count;

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    count = s_device_count;
    memcpy(snap, s_devices, count * sizeof(thread_device_t));
    xSemaphoreGive(s_mutex);

    for (size_t i = 0; i < count; ++i)
    {
        if (snap[i].has_beacon)
            register_observe(instance, snap[i].ipv6_addr, "beacon");
        if (snap[i].has_outdoor)
            register_observe(instance, snap[i].ipv6_addr, "outdoor");
    }
}

// ─── CoAP capability query ────────────────────────────────────────────────────

static void caps_response_cb(void *context, otMessage *message, const otMessageInfo *msg_info, otError result)
{
    char *addr = (char *)context;

    if (result != OT_ERROR_NONE || !message)
    {
        ESP_LOGW(TAG, "Capability query failed for %s: %s", addr, otThreadErrorToString(result));
        free(addr);
        return;
    }

    if (otCoapMessageGetCode(message) != OT_COAP_CODE_CONTENT)
    {
        free(addr);
        return;
    }

    char payload[256] = {};
    uint16_t len = otMessageRead(message, otMessageGetOffset(message), payload, sizeof(payload) - 1);
    payload[len] = '\0';

    bool has_beacon = strstr(payload, "</beacon>") != NULL;
    bool has_outdoor = strstr(payload, "</outdoor>") != NULL;

    ESP_LOGI(TAG, "Capabilities %s: beacon=%d outdoor=%d", addr, has_beacon, has_outdoor);

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    thread_device_t *dev = find_device_by_addr(addr);
    if (dev)
    {
        dev->has_beacon = has_beacon;
        dev->has_outdoor = has_outdoor;
        dev->reachable = true;
    }
    xSemaphoreGive(s_mutex);

    persist_devices();
    update_thread_led();

    char json[192];
    snprintf(json, sizeof(json), "{\"type\":\"thread_capabilities\",\"addr\":\"%s\",\"beacon\":%s,\"outdoor\":%s}",
             addr, has_beacon ? "true" : "false", has_outdoor ? "true" : "false");
    emit(json);

    // Register as observer (RFC 7641) so the device pushes state changes to us
    otInstance *instance = esp_openthread_get_instance();
    if (has_beacon)
        register_observe(instance, addr, "beacon");
    if (has_outdoor)
        register_observe(instance, addr, "outdoor");

    free(addr);
}

static void send_caps_query(otInstance *instance, const char *addr_str)
{
    otMessage *msg = otCoapNewMessage(instance, NULL);
    if (!msg)
        return;

    otCoapMessageInit(msg, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapMessageGenerateToken(msg, OT_COAP_DEFAULT_TOKEN_LENGTH);

    if (otCoapMessageAppendUriPathOptions(msg, ".well-known/core") != OT_ERROR_NONE)
    {
        otMessageFree(msg);
        return;
    }

    otMessageInfo info;
    memset(&info, 0, sizeof(info));
    otIp6AddressFromString(addr_str, &info.mPeerAddr);
    info.mPeerPort = OT_DEFAULT_COAP_PORT;

    char *addr_ctx = strdup(addr_str);
    if (!addr_ctx)
    {
        otMessageFree(msg);
        return;
    }

    if (otCoapSendRequest(instance, msg, &info, caps_response_cb, addr_ctx) != OT_ERROR_NONE)
    {
        otMessageFree(msg);
        free(addr_ctx);
    }
}

// ─── /announce CoAP resource ─────────────────────────────────────────────────

static void announce_handler(void *context, otMessage *message, const otMessageInfo *msg_info)
{
    otInstance *instance = (otInstance *)context;

    if (otCoapMessageGetCode(message) != OT_COAP_CODE_POST)
        return;

    char name[32] = {};
    uint16_t len = otMessageRead(message, otMessageGetOffset(message), name, sizeof(name) - 1);
    name[len] = '\0';

    char addr_str[OT_IP6_ADDRESS_STRING_SIZE] = {};
    otIp6AddressToString(&msg_info->mPeerAddr, addr_str, sizeof(addr_str));

    ESP_LOGI(TAG, "Announce: \"%s\" @ %s", name, addr_str);

    bool is_new = false;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    thread_device_t *dev = find_device_by_addr(addr_str);
    if (!dev && s_device_count < THREAD_MAX_DEVICES)
    {
        dev = &s_devices[s_device_count++];
        memset(dev, 0, sizeof(*dev));
        is_new = true;
    }
    if (dev)
    {
        strlcpy(dev->name, name, sizeof(dev->name));
        strlcpy(dev->ipv6_addr, addr_str, sizeof(dev->ipv6_addr));
        dev->reachable = true;
    }
    xSemaphoreGive(s_mutex);

    if (is_new)
    {
        persist_devices();
        led_status_set_behavior((led_behavior_t){
            .index = THREAD_LED_INDEX,
            .mode = LED_MODE_BLINK,
            .color = {.red = 0, .green = 0, .blue = 64},
            .on_time_ms = 200,
            .off_time_ms = 200,
        });
    }

    char json[192];
    snprintf(json, sizeof(json), "{\"type\":\"thread_device\",\"name\":\"%s\",\"addr\":\"%s\"}", name, addr_str);
    emit(json);

    // We are in the OT task context, so direct call is safe.
    send_caps_query(instance, addr_str);
}

// ─── Commissioner ────────────────────────────────────────────────────────────

static void add_joiner(otInstance *instance)
{
    // NULL = any joiner; timeout 0 = infinite (until disabled or reboot)
    otError err = otCommissionerAddJoiner(instance, NULL, COMMISSIONER_PSK, 0);
    if (err == OT_ERROR_NONE)
        ESP_LOGI(TAG, "Commissioner ready — any joiner accepted with PSKd: %s (infinite timeout)", COMMISSIONER_PSK);
    else
        ESP_LOGE(TAG, "otCommissionerAddJoiner failed: %s", otThreadErrorToString(err));
}

static void commissioner_state_cb(otCommissionerState state, void *context)
{
    const char *state_str = "UNKNOWN";
    switch (state)
    {
    case OT_COMMISSIONER_STATE_DISABLED: state_str = "DISABLED"; break;
    case OT_COMMISSIONER_STATE_PETITION: state_str = "PETITION"; break;
    case OT_COMMISSIONER_STATE_ACTIVE:   state_str = "ACTIVE";   break;
    }
    ESP_LOGI(TAG, "Commissioner State: %s", state_str);

    if (state == OT_COMMISSIONER_STATE_ACTIVE)
        add_joiner((otInstance *)context);
}

static void add_joiner_timer_cb(TimerHandle_t timer)
{
    if (!esp_openthread_lock_acquire(0))
        return;
    ESP_LOGI(TAG, "Add Joiner Timer fired — re-adding joiner");
    add_joiner(esp_openthread_get_instance());
    esp_openthread_lock_release();
}

static void commissioner_joiner_cb(otCommissionerJoinerEvent event, const otJoinerInfo *joiner_info,
                                   const otExtAddress *joiner_id, void *context)
{
    const char *event_str = "UNKNOWN";
    switch (event)
    {
    case OT_COMMISSIONER_JOINER_START:     event_str = "START";     break;
    case OT_COMMISSIONER_JOINER_CONNECTED: event_str = "CONNECTED"; break;
    case OT_COMMISSIONER_JOINER_FINALIZE:  event_str = "FINALIZE";  break;
    case OT_COMMISSIONER_JOINER_END:       event_str = "END";       break;
    case OT_COMMISSIONER_JOINER_REMOVED:   event_str = "REMOVED";   break;
    }

    if (joiner_id) {
        char addr_str[17];
        snprintf(addr_str, sizeof(addr_str), "%02x%02x%02x%02x%02x%02x%02x%02x",
                 joiner_id->m8[0], joiner_id->m8[1], joiner_id->m8[2], joiner_id->m8[3],
                 joiner_id->m8[4], joiner_id->m8[5], joiner_id->m8[6], joiner_id->m8[7]);
        ESP_LOGI(TAG, "Joiner Event: %s (ID: %s)", event_str, addr_str);
    } else {
        ESP_LOGI(TAG, "Joiner Event: %s", event_str);
    }

    // Debounce: rapid REMOVED events (e.g. from NoBufs failures) collapse into one retry.
    if (event == OT_COMMISSIONER_JOINER_REMOVED)
        xTimerReset(s_add_joiner_timer, 0);
}

static void start_commissioner(otInstance *instance)
{
    otError err = otCommissionerStart(instance, commissioner_state_cb, commissioner_joiner_cb, instance);
    if (err != OT_ERROR_NONE)
        ESP_LOGE(TAG, "otCommissionerStart: %s", otThreadErrorToString(err));
}

// ─── CoAP server ─────────────────────────────────────────────────────────────

static void setup_coap(otInstance *instance)
{
    if (s_coap_started)
        return;
    s_coap_started = true;

    otCoapStart(instance, OT_DEFAULT_COAP_PORT);

    s_announce_resource = (otCoapResource){
        .mUriPath = "announce",
        .mHandler = announce_handler,
        .mContext = instance,
        .mNext = NULL,
    };
    otCoapAddResource(instance, &s_announce_resource);

    ESP_LOGI(TAG, "CoAP server: /announce (port %d)", OT_DEFAULT_COAP_PORT);
}

// ─── Become-leader retry timer ────────────────────────────────────────────────

// Fired from a FreeRTOS timer (not OT task context).  Acquire the OT lock
// before touching the instance, and only act if still detached.
static void become_leader_timer_cb(TimerHandle_t timer)
{
    if (!esp_openthread_lock_acquire(0))
        return;

    otInstance *instance = esp_openthread_get_instance();
    otDeviceRole role = otThreadGetDeviceRole(instance);

    if (role == OT_DEVICE_ROLE_DETACHED)
    {
        otError err = otThreadBecomeLeader(instance);
        if (err == OT_ERROR_NONE)
            ESP_LOGI(TAG, "BecomeLeader succeeded after attach timeout");
        else
            ESP_LOGW(TAG, "BecomeLeader retry: %s", otThreadErrorToString(err));
    }

    esp_openthread_lock_release();
}

// ─── State callback ───────────────────────────────────────────────────────────

static void state_changed_cb(otChangedFlags flags, void *context)
{
    otInstance *instance = (otInstance *)context;

    setup_coap(instance);

    if (!(flags & OT_CHANGED_THREAD_ROLE))
        return;

    otDeviceRole role = otThreadGetDeviceRole(instance);
    ESP_LOGI(TAG, "Thread role: %s", otThreadDeviceRoleToString(role));

    if (role == OT_DEVICE_ROLE_DETACHED)
    {
        // The attach scan has already started here (notifier is deferred), so
        // BecomeLeader would return InvalidState.  Start a one-shot timer that
        // fires after the scan window and retries in timer-task context where
        // the OT lock can be acquired cleanly.
        if (s_become_leader_timer)
            xTimerReset(s_become_leader_timer, 0);
    }
    else
    {
        // No longer detached — cancel any pending retry.
        if (s_become_leader_timer)
            xTimerStop(s_become_leader_timer, 0);
    }

    if (role == OT_DEVICE_ROLE_LEADER)
        start_commissioner(instance);

    // Re-register observers for devices loaded from NVS (they may not re-announce)
    if (role == OT_DEVICE_ROLE_LEADER || role == OT_DEVICE_ROLE_ROUTER)
        register_all_observers(instance);
}

// ─── Dataset / network formation ─────────────────────────────────────────────

static void ensure_dataset(otInstance *instance)
{
    otOperationalDataset dataset;

    // otDatasetIsCommissioned only checks NetworkKey/Name/PanId/Channel/ExtPanId.
    // BecomeLeader (with AUTO_INIT=1) additionally requires an Active Timestamp TLV
    // (so that mNetworkTimestamp is valid and IsPartiallyComplete() returns false).
    // A dataset stored by older firmware may have all five commissioning TLVs but
    // no Active Timestamp, causing BecomeLeader to return InvalidState.  Check the
    // timestamp flag explicitly and recreate when it is absent.
    if (otDatasetGetActive(instance, &dataset) == OT_ERROR_NONE && dataset.mComponents.mIsActiveTimestampPresent)
    {
        ESP_LOGI(TAG, "Using existing Thread dataset");
        return;
    }

    ESP_LOGI(TAG, "No complete dataset — creating new Thread network");
    if (otDatasetCreateNewNetwork(instance, &dataset) != OT_ERROR_NONE)
    {
        ESP_LOGE(TAG, "otDatasetCreateNewNetwork failed");
        return;
    }
    if (otDatasetSetActive(instance, &dataset) != OT_ERROR_NONE)
        ESP_LOGE(TAG, "otDatasetSetActive failed");
}

// ─── OpenThread task ─────────────────────────────────────────────────────────

// ─── Neighbor table callback ─────────────────────────────────────────────────

static void neighbor_table_cb(otNeighborTableEvent event, const otNeighborTableEntryInfo *info)
{
    if (event != OT_NEIGHBOR_TABLE_EVENT_CHILD_ADDED && event != OT_NEIGHBOR_TABLE_EVENT_ROUTER_ADDED)
        return;

    char addrs[THREAD_MAX_DEVICES][46];
    size_t count = 0;

    xSemaphoreTake(s_mutex, portMAX_DELAY);
    for (size_t i = 0; i < s_device_count; i++)
    {
        if (!s_devices[i].reachable)
            strlcpy(addrs[count++], s_devices[i].ipv6_addr, 46);
    }
    xSemaphoreGive(s_mutex);

    if (count == 0)
        return;

    otInstance *instance = esp_openthread_get_instance();
    for (size_t i = 0; i < count; i++)
        send_caps_query(instance, addrs[i]);
}

// ─── OpenThread task ─────────────────────────────────────────────────────────

static void thread_task(void *arg)
{
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_openthread_init(&config));

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_OPENTHREAD();
    esp_netif_t *openthread_netif = esp_netif_new(&cfg);
    assert(openthread_netif != NULL);
    ESP_ERROR_CHECK(esp_netif_attach(openthread_netif, esp_openthread_netif_glue_init(&config)));

    // otIp6SetEnabled (and the UDP socket setup it triggers internally) calls
    // esp_openthread_task_switching_lock_release, which asserts the lock is held
    // by the current task.  Acquire it here so that invariant holds during init,
    // then release before handing off to the mainloop (which manages it itself).
    esp_openthread_task_switching_lock_acquire(portMAX_DELAY);

    otInstance *instance = esp_openthread_get_instance();

    otLinkModeConfig mode = {
        .mRxOnWhenIdle = true,
        .mDeviceType = true,
        .mNetworkData = true,
    };
    otThreadSetLinkMode(instance, mode);

    ensure_dataset(instance);
    otSetStateChangedCallback(instance, state_changed_cb, instance);
    otThreadRegisterNeighborTableCallback(instance, neighbor_table_cb);

    ESP_ERROR_CHECK(otIp6SetEnabled(instance, true));
    ESP_ERROR_CHECK(otThreadSetEnabled(instance, true));
    // Call BecomeLeader here, while the lock is held and before the mainloop
    // starts.  The dataset must be complete (Active Timestamp present) for this
    // to succeed — ensure_dataset() guarantees that invariant above.
    {
        otError err = otThreadBecomeLeader(instance);
        if (err != OT_ERROR_NONE)
            ESP_LOGW(TAG, "otThreadBecomeLeader: %s", otThreadErrorToString(err));
    }

    esp_openthread_task_switching_lock_release();

    esp_openthread_launch_mainloop();

    // Clean up
    esp_openthread_netif_glue_deinit();
    esp_netif_destroy(openthread_netif);
    esp_openthread_deinit();
    vTaskDelete(NULL);
}

// ─── CoAP PUT (called from HTTP task) ────────────────────────────────────────

static esp_err_t post_coap_put(const char *addr, const char *uri, const char *payload)
{
    if (!esp_openthread_lock_acquire(portMAX_DELAY))
        return ESP_FAIL;

    otInstance *instance = esp_openthread_get_instance();
    otMessage *msg = otCoapNewMessage(instance, NULL);
    if (!msg)
    {
        esp_openthread_lock_release();
        return ESP_ERR_NO_MEM;
    }

    otCoapMessageInit(msg, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
    otCoapMessageGenerateToken(msg, OT_COAP_DEFAULT_TOKEN_LENGTH);

    if (otCoapMessageAppendUriPathOptions(msg, uri) != OT_ERROR_NONE ||
        otCoapMessageSetPayloadMarker(msg) != OT_ERROR_NONE)
    {
        otMessageFree(msg);
        esp_openthread_lock_release();
        return ESP_FAIL;
    }
    otMessageAppend(msg, payload, (uint16_t)strlen(payload));

    otMessageInfo info;
    memset(&info, 0, sizeof(info));
    otIp6AddressFromString(addr, &info.mPeerAddr);
    info.mPeerPort = OT_DEFAULT_COAP_PORT;

    if (otCoapSendRequest(instance, msg, &info, NULL, NULL) != OT_ERROR_NONE)
        otMessageFree(msg);

    esp_openthread_lock_release();
    return ESP_OK;
}

// ─── Caps query (for calls outside OT context) ───────────────────────────────

static esp_err_t post_caps_query(const char *addr)
{
    if (!esp_openthread_lock_acquire(portMAX_DELAY))
        return ESP_FAIL;
    send_caps_query(esp_openthread_get_instance(), addr);
    esp_openthread_lock_release();
    return ESP_OK;
}

// ─── Public API ──────────────────────────────────────────────────────────────

esp_err_t thread_manager_init(thread_event_cb_t event_cb)
{
    s_event_cb = event_cb;
    s_device_count = 0;
    s_group_count = 0;
    s_coap_started = false;

    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex)
        return ESP_ERR_NO_MEM;

    s_become_leader_timer = xTimerCreate("ot_leader", pdMS_TO_TICKS(15000), pdFALSE, NULL, become_leader_timer_cb);
    if (!s_become_leader_timer)
    {
        vSemaphoreDelete(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    s_add_joiner_timer = xTimerCreate("ot_joiner", pdMS_TO_TICKS(10000), pdFALSE, NULL, add_joiner_timer_cb);
    if (!s_add_joiner_timer)
    {
        xTimerDelete(s_become_leader_timer, 0);
        vSemaphoreDelete(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    esp_err_t netif_err = esp_netif_init();
    if (netif_err != ESP_OK && netif_err != ESP_ERR_INVALID_STATE)
        ESP_RETURN_ON_ERROR(netif_err, TAG, "esp_netif_init failed");

    esp_err_t loop_err = esp_event_loop_create_default();
    if (loop_err != ESP_OK && loop_err != ESP_ERR_INVALID_STATE)
        ESP_RETURN_ON_ERROR(loop_err, TAG, "esp_event_loop_create_default failed");

    esp_vfs_eventfd_config_t eventfd_config = {.max_fds = 5};
    ESP_RETURN_ON_ERROR(esp_vfs_eventfd_register(&eventfd_config), TAG, "Failed to register VFS eventfd");

    load_devices();
    load_groups();

    BaseType_t ret = xTaskCreate(thread_task, "thread_task", 10240, NULL, 5, NULL);
    if (ret != pdPASS)
    {
        vSemaphoreDelete(s_mutex);
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

const thread_device_t *thread_manager_get_devices(size_t *count)
{
    *count = s_device_count;
    return s_devices;
}

esp_err_t thread_manager_add_device(const char *name, const char *ipv6_addr)
{
    if (!name || !ipv6_addr || !ipv6_addr[0])
        return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    thread_device_t *dev = find_device_by_addr(ipv6_addr);
    if (dev)
    {
        strlcpy(dev->name, name, sizeof(dev->name));
        xSemaphoreGive(s_mutex);
        persist_devices();
        return ESP_OK;
    }

    if (s_device_count >= THREAD_MAX_DEVICES)
    {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    dev = &s_devices[s_device_count++];
    memset(dev, 0, sizeof(*dev));
    strlcpy(dev->name, name, sizeof(dev->name));
    strlcpy(dev->ipv6_addr, ipv6_addr, sizeof(dev->ipv6_addr));

    xSemaphoreGive(s_mutex);

    persist_devices();
    post_caps_query(ipv6_addr);

    char json[192];
    snprintf(json, sizeof(json), "{\"type\":\"thread_device\",\"name\":\"%s\",\"addr\":\"%s\"}", name, ipv6_addr);
    emit(json);

    return ESP_OK;
}

esp_err_t thread_manager_remove_device(const char *ipv6_addr)
{
    if (!ipv6_addr)
        return ESP_ERR_INVALID_ARG;

    char group_addrs[THREAD_MAX_GROUPS][46];
    uint8_t group_count = 0;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    size_t idx = THREAD_MAX_DEVICES;
    for (size_t i = 0; i < s_device_count; ++i)
    {
        if (strcmp(s_devices[i].ipv6_addr, ipv6_addr) == 0)
        {
            idx = i;
            break;
        }
    }

    if (idx == THREAD_MAX_DEVICES)
    {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    // Remove device from all groups, collect multicast addrs for CoAP leave
    for (size_t g = 0; g < s_group_count; ++g)
    {
        thread_group_t *grp = &s_groups[g];
        for (uint8_t m = 0; m < grp->member_count; ++m)
        {
            if (strcmp(grp->member_addrs[m], ipv6_addr) == 0)
            {
                strlcpy(group_addrs[group_count++], grp->multicast_addr, 46);
                uint8_t remaining = grp->member_count - m - 1;
                if (remaining > 0)
                    memmove(grp->member_addrs[m], grp->member_addrs[m + 1], remaining * sizeof(grp->member_addrs[0]));
                grp->member_count--;
                break;
            }
        }
    }

    uint8_t last = (uint8_t)(s_device_count - idx - 1);
    if (last > 0)
        memmove(&s_devices[idx], &s_devices[idx + 1], last * sizeof(thread_device_t));
    s_device_count--;

    xSemaphoreGive(s_mutex);

    // Send group leave to all groups the device was in
    char payload[64];
    for (uint8_t g = 0; g < group_count; ++g)
    {
        snprintf(payload, sizeof(payload), "%s,0", group_addrs[g]);
        post_coap_put(ipv6_addr, "group", payload);
    }

    persist_devices();
    if (group_count > 0)
        persist_groups();

    char json[128];
    snprintf(json, sizeof(json), "{\"type\":\"thread_device_removed\",\"addr\":\"%s\"}", ipv6_addr);
    emit(json);

    return ESP_OK;
}

esp_err_t thread_manager_set_resource(const char *ipv6_addr, const char *resource, bool on)
{
    return post_coap_put(ipv6_addr, resource, on ? "1" : "0");
}

const thread_group_t *thread_manager_get_groups(size_t *count)
{
    *count = s_group_count;
    return s_groups;
}

esp_err_t thread_manager_add_group(const char *name, const char *multicast_addr)
{
    if (!name || !multicast_addr || !multicast_addr[0])
        return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    if (find_group_by_addr(multicast_addr))
    {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_STATE;
    }

    if (s_group_count >= THREAD_MAX_GROUPS)
    {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    thread_group_t *grp = &s_groups[s_group_count++];
    memset(grp, 0, sizeof(*grp));
    strlcpy(grp->name, name, sizeof(grp->name));
    strlcpy(grp->multicast_addr, multicast_addr, sizeof(grp->multicast_addr));

    xSemaphoreGive(s_mutex);

    persist_groups();

    char json[192];
    snprintf(json, sizeof(json), "{\"type\":\"thread_group_added\",\"name\":\"%s\",\"addr\":\"%s\"}", name,
             multicast_addr);
    emit(json);

    return ESP_OK;
}

esp_err_t thread_manager_delete_group(const char *multicast_addr)
{
    if (!multicast_addr)
        return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    size_t idx = THREAD_MAX_GROUPS;
    for (size_t i = 0; i < s_group_count; ++i)
    {
        if (strcmp(s_groups[i].multicast_addr, multicast_addr) == 0)
        {
            idx = i;
            break;
        }
    }

    if (idx == THREAD_MAX_GROUPS)
    {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    // Snapshot members for CoAP leave (sent after mutex release)
    char member_addrs[THREAD_GROUP_MAX_MEMBERS][46];
    uint8_t member_count = s_groups[idx].member_count;
    memcpy(member_addrs, s_groups[idx].member_addrs, sizeof(member_addrs));

    uint8_t last = (uint8_t)(s_group_count - idx - 1);
    if (last > 0)
        memmove(&s_groups[idx], &s_groups[idx + 1], last * sizeof(thread_group_t));
    s_group_count--;

    xSemaphoreGive(s_mutex);

    // Notify all former members to leave
    char payload[64];
    snprintf(payload, sizeof(payload), "%s,0", multicast_addr);
    for (uint8_t m = 0; m < member_count; ++m)
        post_coap_put(member_addrs[m], "group", payload);

    persist_groups();

    char json[128];
    snprintf(json, sizeof(json), "{\"type\":\"thread_group_removed\",\"addr\":\"%s\"}", multicast_addr);
    emit(json);

    return ESP_OK;
}

esp_err_t thread_manager_assign_device(const char *device_addr, const char *multicast_addr)
{
    if (!device_addr || !multicast_addr)
        return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    thread_group_t *grp = find_group_by_addr(multicast_addr);
    if (!grp)
    {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    for (uint8_t m = 0; m < grp->member_count; ++m)
    {
        if (strcmp(grp->member_addrs[m], device_addr) == 0)
        {
            xSemaphoreGive(s_mutex);
            return ESP_OK; // idempotent
        }
    }

    if (grp->member_count >= THREAD_GROUP_MAX_MEMBERS)
    {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NO_MEM;
    }

    strlcpy(grp->member_addrs[grp->member_count++], device_addr, 46);

    xSemaphoreGive(s_mutex);

    char payload[64];
    snprintf(payload, sizeof(payload), "%s,1", multicast_addr);
    post_coap_put(device_addr, "group", payload);

    persist_groups();

    char json[192];
    snprintf(json, sizeof(json), "{\"type\":\"thread_group_assigned\",\"device\":\"%s\",\"group\":\"%s\"}", device_addr,
             multicast_addr);
    emit(json);

    return ESP_OK;
}

esp_err_t thread_manager_unassign_device(const char *device_addr, const char *multicast_addr)
{
    if (!device_addr || !multicast_addr)
        return ESP_ERR_INVALID_ARG;

    xSemaphoreTake(s_mutex, portMAX_DELAY);

    thread_group_t *grp = find_group_by_addr(multicast_addr);
    if (!grp)
    {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    bool found = false;
    for (uint8_t m = 0; m < grp->member_count; ++m)
    {
        if (strcmp(grp->member_addrs[m], device_addr) == 0)
        {
            uint8_t remaining = grp->member_count - m - 1;
            if (remaining > 0)
                memmove(grp->member_addrs[m], grp->member_addrs[m + 1], remaining * sizeof(grp->member_addrs[0]));
            grp->member_count--;
            found = true;
            break;
        }
    }

    xSemaphoreGive(s_mutex);

    if (!found)
        return ESP_ERR_NOT_FOUND;

    char payload[64];
    snprintf(payload, sizeof(payload), "%s,0", multicast_addr);
    post_coap_put(device_addr, "group", payload);

    persist_groups();

    char json[192];
    snprintf(json, sizeof(json), "{\"type\":\"thread_group_unassigned\",\"device\":\"%s\",\"group\":\"%s\"}",
             device_addr, multicast_addr);
    emit(json);

    return ESP_OK;
}

esp_err_t thread_manager_group_command(const char *multicast_addr, const char *resource, bool on)
{
    return post_coap_put(multicast_addr, resource, on ? "1" : "0");
}
