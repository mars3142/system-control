#pragma once

/**
 * @file iris_internal.h
 * @brief Shared state, types, and internal function declarations for the Iris component.
 *
 * This header is NOT part of the public API. It is included only by the Iris
 * source files (iris.c, iris_storage.c, iris_coap.c, iris_discovery.c,
 * iris_master.c, iris_inventory.c).
 */

#include "iris/iris.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <cJSON.h>

#if defined(CONFIG_IRIS_ENABLED)
#include <esp_openthread.h>
#include <esp_openthread_types.h>
#include <esp_vfs_eventfd.h>
#include <openthread/coap.h>
#include <openthread/commissioner.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/thread.h>
#include <openthread/thread_ftd.h>

/* =========================================================================
 * SPIFFS storage constants and types
 * ========================================================================= */

#define IRIS_STORE_PATH    "/spiffs/iris_devices.bin"
#define IRIS_STORE_MAGIC   0x49524953u  /* "IRIS" */
#define IRIS_STORE_VERSION 1

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t version;
    uint16_t count;
} iris_store_header_t;

/* =========================================================================
 * Master election state
 * ========================================================================= */

typedef enum {
    IRIS_MASTER_INITIALIZING,
    IRIS_MASTER_ACTIVE,
    IRIS_MASTER_STANDBY,
} iris_master_state_t;

/* =========================================================================
 * Scan cache
 * ========================================================================= */

#define IRIS_SCAN_CACHE_MAX 8

/* =========================================================================
 * Shared state — defined in iris.c, accessed by all sub-modules
 * ========================================================================= */

extern iris_device_t        s_paired[CONFIG_IRIS_MAX_DEVICES];
extern int                  s_paired_count;
extern SemaphoreHandle_t    s_mutex;
extern TaskHandle_t         s_inventory_task_handle;
extern TaskHandle_t         s_master_task_handle;

extern iris_master_state_t  s_master_state;
extern bool                 s_master_is_us;

extern iris_device_t        s_scan_cache[IRIS_SCAN_CACHE_MAX];
extern int                  s_scan_count;
extern volatile bool        s_discovery_active;

/* =========================================================================
 * Internal function declarations
 * ========================================================================= */

/* iris.c */
int  find_device_index(const uint8_t eui64[IRIS_EUI64_LEN]);

/* iris_storage.c */
void spiffs_save(void);
void spiffs_load(void);

/* iris_coap.c */
bool eui64_to_ml_eid(const uint8_t eui64[IRIS_EUI64_LEN], otIp6Address *addr);
bool coap_get(const otIp6Address *addr, const char *resource,
              char *out_buf, size_t out_len);
bool coap_post(const otIp6Address *addr, const char *resource,
               const char *payload);

/* iris_discovery.c */
void iris_neighbor_callback(otNeighborTableEvent event,
                             const otNeighborTableEntryInfo *info);
void joiner_callback(otCommissionerJoinerEvent event,
                     const otJoinerInfo *info,
                     const otExtAddress *eui64,
                     void *ctx);

/* iris_master.c — the task function; started by iris_start_inventory_task */
void iris_master_task(void *arg);

/* iris_inventory.c — the task function */
void iris_inventory_task(void *arg);

#endif /* CONFIG_IRIS_ENABLED */
