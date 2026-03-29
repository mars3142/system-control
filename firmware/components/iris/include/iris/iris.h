#pragma once

/**
 * @file iris.h
 * @brief Iris — Thread network device manager for ESP32-C6
 *
 * Iris manages paired Thread devices (ESP32-H2 clients) on the model railway
 * control system. It handles:
 *  - Device provisioning via Thread Commissioner + Joiner flow
 *  - Capability discovery via CoAP GET /capabilities
 *  - State polling via CoAP GET /state (background inventory task)
 *  - Device control via CoAP POST /toggle (unicast and multicast)
 *  - Master/Backup election so two C6 units auto-elect who controls the network
 *  - Persistence via SPIFFS binary file (see README-thread.md §6)
 *
 * See README-thread.md for the full protocol reference and H2 implementation guide.
 */

#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Capability and State bitmasks
 * These values MUST match the H2 firmware definition exactly.
 * ========================================================================= */

/** @defgroup iris_caps Capability bitmask (what the device supports) */
/** @{ */
#define IRIS_CAP_INNER_LIGHT   (1u << 0)  /**< Innenbeleuchtung */
#define IRIS_CAP_OUTER_LIGHT   (1u << 1)  /**< Außenbeleuchtung */
#define IRIS_CAP_MOVEMENT      (1u << 2)  /**< Bewegung (Oben / Unten) */
/** @} */

/** @defgroup iris_state State bitmask (current value of each capability) */
/** @{ */
#define IRIS_STATE_INNER_LIGHT (1u << 0)  /**< 1 = on,   0 = off  */
#define IRIS_STATE_OUTER_LIGHT (1u << 1)  /**< 1 = on,   0 = off  */
#define IRIS_STATE_MOVEMENT    (1u << 2)  /**< 1 = Oben, 0 = Unten */
/** @} */

/* =========================================================================
 * Device data structures
 * ========================================================================= */

#define IRIS_DEVICE_NAME_MAX  32
#define IRIS_EUI64_LEN        8

/**
 * @brief Persisted portion of a device record (written to SPIFFS as-is).
 *
 * Size: 44 bytes (packed). Layout documented in README-thread.md §6.2.
 */
typedef struct __attribute__((packed)) {
    uint8_t eui64[IRIS_EUI64_LEN];       /**< Hardware EUI-64 identifier     */
    char    name[IRIS_DEVICE_NAME_MAX];  /**< Display name (null-terminated)  */
    uint8_t capabilities;                /**< IRIS_CAP_* bitmask              */
    uint8_t state;                       /**< IRIS_STATE_* bitmask (last known) */
    uint8_t _pad[2];                     /**< Alignment padding — must be 0   */
} iris_device_persisted_t;

/**
 * @brief Full device record including runtime-only fields.
 *
 * The `p` sub-struct is the only part written to / read from SPIFFS.
 * Runtime fields are initialised to safe defaults on load and updated
 * by the inventory task.
 */
typedef struct {
    iris_device_persisted_t p;       /**< Persisted data                         */
    bool    online;                  /**< true after at least one successful poll */
    uint8_t failed_polls;            /**< Consecutive CoAP errors / timeouts     */
} iris_device_t;

/* =========================================================================
 * Lifecycle
 * ========================================================================= */

/**
 * @brief Initialise the Iris component.
 *
 * Loads the paired device list from SPIFFS, initialises OpenThread as Border
 * Router, and starts the Master election task. Call once from app_task after
 * wifi_manager_init().
 *
 * @return ESP_OK on success, or an error code.
 */
esp_err_t iris_init(void);

/**
 * @brief Start the background inventory task.
 *
 * Spawns a FreeRTOS task (priority tskIDLE_PRIORITY+2, 4 KB stack) that:
 *  - Runs an initial discovery sweep on startup
 *  - Periodically polls all paired devices via CoAP GET /state
 *  - Re-runs discovery every IRIS_DISCOVERY_INTERVAL_CYCLES poll cycles
 * Call once after iris_init().
 */
void iris_start_inventory_task(void);

/**
 * @brief Run an active network discovery sweep (blocking).
 *
 * Sends a multicast CoAP NON GET /discover to ff03::1 and collects responses
 * for IRIS_DISCOVERY_WINDOW_MS milliseconds. For each responding device:
 *  - If already paired: marks the device online and updates its state.
 *  - If not yet paired: adds it to the scan cache so the OLED
 *    provisioning menu ("neue Geräte") can offer it for pairing.
 *
 * This allows the C6 to rediscover devices that are already in the Thread
 * network after a firmware flash that wiped the SPIFFS paired-device list.
 *
 * Must be called from a FreeRTOS task context (blocks for the collection
 * window). Do NOT call from the OpenThread mainloop task.
 */
void iris_run_discovery(void);

/* =========================================================================
 * Device discovery and provisioning
 * ========================================================================= */

/**
 * @brief Return cached list of discoverable (unpaired) Thread joiners.
 *
 * Devices that have called otJoinerStart() but have not yet been paired appear
 * here. The list is filled by the Commissioner joiner callback and cached in
 * memory; this function returns from the cache without blocking.
 *
 * @param[out] out      Buffer to receive device records.
 * @param[in]  max      Maximum number of records to return.
 * @return Number of devices written to @p out.
 */
int iris_scan(iris_device_t *out, int max);

/**
 * @brief Provision a discovered device into the Thread network.
 *
 * Calls otCommissionerAddJoiner() with the project PSKd, waits for the join
 * confirmation callback, then performs a CoAP GET /capabilities exchange.
 * On success, adds the device to the in-memory list and persists to SPIFFS.
 *
 * @param[in] eui64  Device EUI-64 (from iris_scan result).
 * @param[in] name   Human-readable display name (max IRIS_DEVICE_NAME_MAX-1 chars).
 * @return ESP_OK on success.
 */
esp_err_t iris_pair(const uint8_t eui64[IRIS_EUI64_LEN], const char *name);

/* =========================================================================
 * Paired device management
 * ========================================================================= */

/**
 * @brief Return the list of all paired devices.
 *
 * Copies from the in-memory device list. Thread-safe (mutex-protected).
 *
 * @param[out] out  Buffer to receive device records.
 * @param[in]  max  Maximum records.
 * @return Number of paired devices written to @p out.
 */
int iris_get_paired(iris_device_t *out, int max);

/**
 * @brief Toggle one capability on a specific device (CoAP unicast).
 *
 * Sends CoAP POST /toggle {"cap": cap} to the device's Thread RLOC16 address.
 * No-op if the device is currently offline.
 *
 * @param[in] eui64  Target device EUI-64.
 * @param[in] cap    Exactly one IRIS_CAP_* bit (e.g. IRIS_CAP_INNER_LIGHT).
 * @return ESP_OK, ESP_ERR_NOT_FOUND if device unknown, or a CoAP error.
 */
esp_err_t iris_toggle(const uint8_t eui64[IRIS_EUI64_LEN], uint8_t cap);

/**
 * @brief Set one capability to an explicit state on ALL devices (CoAP multicast).
 *
 * Sends CoAP POST /set {"cap": cap, "state": on} to the Thread Realm-Local
 * All-Nodes multicast address ff03::1. All H2 devices that support @p cap
 * will apply the requested state, regardless of their current state.
 *
 * Use explicit on/off instead of toggle so that a "all lights on" command
 * does not accidentally turn off devices that are already on.
 *
 * @param[in] cap  Exactly one IRIS_CAP_* bit.
 * @param[in] on   true = activate, false = deactivate.
 * @return ESP_OK on successful send (delivery is best-effort multicast).
 */
esp_err_t iris_set_all(uint8_t cap, bool on);

/**
 * @brief Rename a paired device and persist the change to SPIFFS.
 *
 * @param[in] eui64     Target device EUI-64.
 * @param[in] new_name  New display name (max IRIS_DEVICE_NAME_MAX-1 chars).
 * @return ESP_OK, or ESP_ERR_NOT_FOUND.
 */
esp_err_t iris_rename(const uint8_t eui64[IRIS_EUI64_LEN], const char *new_name);

/**
 * @brief Remove a paired device and persist the change to SPIFFS.
 *
 * Removes the device from the in-memory list and rewrites SPIFFS. Does not
 * attempt to disconnect the device from the Thread network — the device will
 * simply be ignored on future inventory polls.
 *
 * @param[in] eui64  Target device EUI-64.
 * @return ESP_OK, or ESP_ERR_NOT_FOUND.
 */
esp_err_t iris_unpair(const uint8_t eui64[IRIS_EUI64_LEN]);

/* =========================================================================
 * Capability query helpers
 * ========================================================================= */

/**
 * @brief Returns true if at least one paired device supports the given capability.
 *
 * Used to decide whether to show multicast toggle items in the menu.
 *
 * @param[in] cap  One IRIS_CAP_* bit.
 */
bool iris_any_has_cap(uint8_t cap);

/* =========================================================================
 * Master / Backup state
 * ========================================================================= */

/**
 * @brief Returns true if this unit is currently the active Master.
 *
 * The Master is the Commissioner and the only unit that can provision and
 * control devices. The Backup monitors the network but does not commission.
 * The OLED menu hides device management options when this returns false.
 */
bool iris_is_master(void);

/**
 * @brief Returns the configured Master election priority for this unit.
 *
 * Corresponds to CONFIG_IRIS_MASTER_PRIORITY. Higher = preferred Master.
 */
uint8_t iris_get_priority(void);

/* =========================================================================
 * Utility
 * ========================================================================= */

/**
 * @brief Convert a binary EUI-64 to a 16-character hex string.
 *
 * @param[in]  eui64   8-byte binary EUI-64.
 * @param[out] out     Caller-provided buffer, must be at least 17 bytes.
 * @param[in]  len     Size of @p out in bytes.
 */
void iris_eui64_to_str(const uint8_t eui64[IRIS_EUI64_LEN], char *out, size_t len);

/**
 * @brief Parse a 16-character hex string into a binary EUI-64.
 *
 * @param[in]  str   Null-terminated hex string (exactly 16 hex chars).
 * @param[out] eui64 Output buffer (8 bytes).
 * @return true on success, false if the string is invalid.
 */
bool iris_str_to_eui64(const char *str, uint8_t eui64[IRIS_EUI64_LEN]);

#ifdef __cplusplus
}
#endif
