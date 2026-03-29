# Thread Network — Architecture & Protocol Reference

This document describes the Thread network integration for the system-control firmware and serves as the primary reference for implementing compatible ESP32-H2 client devices.

---

## 1. Network Architecture

```
┌──────────────────────────────────────────────────────┐
│                  Thread Mesh Network                 │
│                                                      │
│   [ESP32-C6 Master]          [ESP32-C6 Backup]       │
│   Border Router              Standby                 │
│   Commissioner               (no Commissioner)       │
│        │                           │                 │
│        └────────────┬──────────────┘                 │
│                     │                                │
│          ┌──────────┼──────────┐                     │
│          │          │          │                     │
│      [H2 #1]    [H2 #2]    [H2 #N]                   │
│      FTD          FTD        FTD                     │
└──────────────────────────────────────────────────────┘
         │
    WiFi / Ethernet (Border Router uplink)
```

**Roles:**
- **ESP32-C6 (Master):** Thread Border Router + Commissioner. Manages device provisioning, sends commands, runs inventory polling. Only one C6 is Master at any time.
- **ESP32-C6 (Backup):** Standby. Monitors the Thread network but does not commission or control devices. Automatically becomes Master if the primary fails.
- **ESP32-H2:** Full Thread Device (FTD). Hosts a CoAP server exposing capabilities and accepting control commands.

**ESP-IDF component:** `openthread` (OpenThread 1.3, enabled via `CONFIG_OPENTHREAD_ENABLED=y`)

---

## 2. Capability Bitmask

The capability and state bitmasks are shared between C6 firmware (Iris component) and H2 firmware. Both sides **must** use identical bit definitions.

```c
/* Capabilities — what the device can do */
#define IRIS_CAP_INNER_LIGHT   (1u << 0)  /* Innenbeleuchtung */
#define IRIS_CAP_OUTER_LIGHT   (1u << 1)  /* Außenbeleuchtung */
#define IRIS_CAP_MOVEMENT      (1u << 2)  /* Bewegung (Oben/Unten) */

/* State — current value of each capability */
#define IRIS_STATE_INNER_LIGHT (1u << 0)  /* 1 = on,   0 = off  */
#define IRIS_STATE_OUTER_LIGHT (1u << 1)  /* 1 = on,   0 = off  */
#define IRIS_STATE_MOVEMENT    (1u << 2)  /* 1 = Oben, 0 = Unten */
```

Example: A wagon with interior lighting and a movement mechanism has `capabilities = 0x05` (bits 0 and 2 set).

---

## 3. Device States and Lifecycle

A device can be in one of three states from the C6's perspective:

| State | Description | Source | C6 Action |
|-------|-------------|--------|-----------|
| **New Joiner** | Never provisioned; wants to join via Commissioner flow | `otCommissionerJoinerCallback` | Show in "neue Geräte" menu for manual "Aufnehmen" |
| **Rejoined** | Previously provisioned and in the Thread network, but C6 lost its SPIFFS record (e.g. after firmware flash) | `GET /discover` response | Auto-restore to `iris_devices.bin` immediately, no user action required |
| **Paired** | In `iris_devices.bin`, actively polled | SPIFFS + inventory task | Normal operation |

### 3.1 Prerequisites
- H2 device must be flashed with firmware that starts the Thread Joiner.
- C6 Master must be active as Commissioner.
- Both devices must know the **PSKd** (Pre-Shared Key for device). Currently a project-wide shared secret configured in `Kconfig` (`CONFIG_IRIS_JOINER_PSKD`).

### 3.2 New Device Joining Flow

```
H2 Firmware                       C6 Master (Iris)
     │                                   │
     │  (power on, Thread not joined)    │
     │                                   │
     │── otJoinerStart(pskd) ──────────► │
     │                                   │  otCommissionerAddJoiner(eui64, pskd)
     │                                   │  (C6 allows this EUI-64 to join)
     │◄── DTLS handshake ───────────────►│
     │◄── Commissioner sets Network Key ─│
     │                                   │
     │  (H2 is now on Thread network)    │
     │                                   │
     │◄── CoAP GET /capabilities ────────│  C6 queries H2 capabilities
     │─── {"caps": <bitmask>} ──────────►│
     │                                   │  C6 stores device in SPIFFS
     │                                   │  C6 shows device in "externe Geräte"
```

### 3.3 Rejoined Device — Auto-Restore Flow

When the C6 boots after a firmware flash (SPIFFS wiped), all previously paired devices
are still in the Thread network. The discovery sweep finds them automatically:

```
C6 boots (iris_devices.bin empty)
     │
     │── NON GET /discover ─────────────► ff03::1
     │                                         │
     │                           H2 (in network, was previously paired)
     │◄── {"eui64":"..","caps":3,"state":1} ───│
     │
     C6: EUI-64 not in paired list
     → auto-restore: add to iris_devices.bin
     → device appears in "externe Geräte" immediately
     (no user interaction required)
```

### 3.4 Online Detection

Two mechanisms work in parallel for instant online detection:

1. **Neighbor table callback** (`otThreadRegisterNeighborTableCallback`): fires immediately when a device joins or rejoins the Thread network at the Link layer. Sets `online=true` for known devices and wakes the inventory task via `xTaskNotify` for an immediate state poll.

2. **Discovery sweep** (`GET /discover` multicast): runs on boot and every `IRIS_DISCOVERY_INTERVAL_CYCLES` inventory cycles. Finds both known and unknown devices.

### 3.5 H2 Implementation Requirements

The H2 firmware must implement:

```c
// 1. Start Thread Joiner on boot (if not already joined)
otJoinerStart(instance, PSKD, NULL, "Vendor", "Model", "1.0", NULL,
              joiner_callback, NULL);

// 2. On successful join, register as CoAP server
otCoapStart(instance, OT_DEFAULT_COAP_PORT);  // port 5683

// 3. Register CoAP resources:
//    GET  /capabilities  — static hardware capabilities
//    GET  /state         — current state bitmask
//    GET  /discover      — discovery response (multicast)
//    POST /toggle        — unicast toggle one capability
//    POST /set           — multicast explicit state set
```

---

## 4. CoAP Protocol

All communication uses **CoAP over UDP** (RFC 7252). Port **5683** (default CoAP port).

JSON is used for payloads. All fields are integer bitmasks matching the definitions in section 2.

### 4.1 GET /capabilities (Unicast)

Returns the device's static capability bitmask (does not change after boot).

**Request:** `GET coap://[<device_ml_eid>]/capabilities`

**Response:**
```json
{"caps": 5}
```
| Field | Type | Description |
|-------|------|-------------|
| `caps` | uint8 | `IRIS_CAP_*` bitmask |

### 4.2 GET /state (Unicast)

Returns the current state of all capabilities.

**Request:** `GET coap://[<device_ml_eid>]/state`

**Response:**
```json
{"state": 3}
```
| Field | Type | Description |
|-------|------|-------------|
| `state` | uint8 | `IRIS_STATE_*` bitmask |

### 4.3 GET /discover (Multicast)

Used by the C6 Master to find all Iris-capable devices in the Thread network.
Sent as CON or NON to `ff03::1`; every H2 that has completed the Joiner flow responds.

**Request:** `GET coap://[ff03::1]/discover`

**Response** (each H2 sends one response):
```json
{"eui64": "aabbccddeeff0011", "caps": 3, "state": 1, "name": "Wagen 42"}
```
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `eui64` | string | yes | 16-hex-char EUI-64 identifier |
| `caps`  | uint8  | yes | `IRIS_CAP_*` bitmask |
| `state` | uint8  | yes | `IRIS_STATE_*` bitmask (current) |
| `name`  | string | no  | Stored display name (if H2 persists it); used by C6 when auto-restoring |

**C6 behavior on receiving responses:**
- EUI-64 in `iris_devices.bin` → mark online, update state
- EUI-64 NOT in `iris_devices.bin` → auto-add to `iris_devices.bin` (rejoined device)

**Note:** H2 must subscribe to `ff03::1` to receive this request (see section 7.3).

### 4.4 POST /toggle (Unicast)

Toggles one capability on the addressed device. Intended for 1:1 control from the OLED menu.

**Request:** `POST coap://[<device_ml_eid>]/toggle`
```json
{"cap": 1}
```
| Field | Type | Description |
|-------|------|-------------|
| `cap` | uint8 | Exactly one `IRIS_CAP_*` bit set |

**Response:** `2.04 Changed` (no body)

**Behavior on H2:**
- `IRIS_CAP_INNER_LIGHT`: toggle inner light on/off
- `IRIS_CAP_OUTER_LIGHT`: toggle outer light on/off
- `IRIS_CAP_MOVEMENT`: toggle between Oben (1) and Unten (0)

### 4.5 POST /set (Multicast — Explicit State)

Sets one capability to an **explicit** on/off state on all devices simultaneously.
Sent as NON to `ff03::1`.

**Do not use toggle for multicast.** A toggle command sent to multiple devices would turn
off devices that are already in the target state. `/set` is idempotent: all devices
end up in the same state regardless of their current state.

**Request:** `POST coap://[ff03::1]/set`
```json
{"cap": 1, "state": 1}
```
| Field | Type | Description |
|-------|------|-------------|
| `cap`   | uint8 | Exactly one `IRIS_CAP_*` bit set |
| `state` | uint8 | `1` = activate, `0` = deactivate |

**Response:** none (NON, best-effort delivery)

**Behavior on H2:**
- If `cap & MY_CAPS`: apply the requested state directly (do NOT toggle)
- If `cap` not in `MY_CAPS`: ignore

---

## 5. Master/Backup Election Protocol

Two C6 devices can be on the same Thread network. Only one is **Master** (active Commissioner + controller). The other is **Standby** (Backup). Election is automatic and priority-based.

### 5.1 Priority

Each device has a configured priority: `CONFIG_IRIS_MASTER_PRIORITY` (Kconfig, default 1). A higher number means higher preference for Master role. The intended Primary device should be configured with a higher priority (e.g., 2).

### 5.2 State Machine

```
       ┌─────────────────────────────────────────┐
       │             INITIALIZING                │
       │  (random jitter 0–1s, then probe)       │
       └────────────────────┬────────────────────┘
                            │
             Multicast GET /master_probe
                            │
              ┌─────────────┴─────────────┐
              │                           │
     No response or              Response with higher
     lower-prio response         priority received
              │                           │
              ▼                           ▼
    ┌──────────────────┐      ┌──────────────────────┐
    │      MASTER      │      │       STANDBY        │
    │ Commissioner on  │      │ Commissioner off     │
    │ Heartbeat every  │      │ Monitor heartbeats   │
    │ 5s via multicast │      │ from Master          │
    └────────┬─────────┘      └──────────┬───────────┘
             │                           │
    Higher-prio peer           Heartbeat timeout
    sends heartbeat            (15s no heartbeat)
             │                           │
             ▼                           ▼
    ┌──────────────────┐      ┌──────────────────────┐
    │     STANDBY      │      │    INITIALIZING      │
    │ (yield, become   │      │ (re-election)        │
    │  backup)         │      └──────────────────────┘
    └──────────────────┘
```

### 5.3 CoAP Election Messages (Multicast `ff03::1`)

| Method | Resource | Description |
|--------|----------|-------------|
| `GET`  | `/master_probe` | Query: who is Master? |
| `PUT`  | `/master_heartbeat` | Regular keepalive from Master |
| `PUT`  | `/master_yield` | Backup acknowledges Master transfer |

**`GET /master_probe` response:**
```json
{"priority": 2, "master": true}
```

**`PUT /master_heartbeat` body:**
```json
{"priority": 2}
```

**`PUT /master_yield` body:**
```json
{"priority": 1}
```

### 5.4 Failback (Primary Returns)

When the Primary (higher priority) returns after a failure:

```
Primary (prio=2)             Backup (prio=1, currently MASTER)
      │                               │
      │── PUT /master_heartbeat ─────►│
      │   {"priority": 2}             │
      │                               │  (sees higher prio → yield)
      │◄── PUT /master_yield ─────────│
      │    {"priority": 1}            │
      │                               │  Backup: Commissioner OFF → STANDBY
      │  Primary: Commissioner ON     │
      │  → MASTER                     │
```

No user interaction required. The OLED display on the Backup shows "BACKUP" after yielding.

---

## 6. SPIFFS Storage Format

Paired devices are stored in `/spiffs/iris_devices.bin` as raw binary (no JSON overhead).

### 6.1 File Layout

```
Offset  Size  Field
──────  ────  ─────────────────────────────────────────────
0       4     magic = 0x49524953 ("IRIS", little-endian)
4       2     version = 1
6       2     count (number of stored devices)
8       N×44  array of iris_device_persisted_t[count]
```

### 6.2 `iris_device_persisted_t` (44 bytes, packed)

```
Offset  Size  Field
──────  ────  ─────────────────────────────────────────────
0       8     eui64[8]        — hardware EUI-64
8       32    name[32]        — display name, null-terminated
40      1     capabilities    — IRIS_CAP_* bitmask
41      1     state           — IRIS_STATE_* bitmask (last known)
42      2     _pad            — alignment padding, set to 0
```

**Runtime fields** (`online`, `failed_polls`) are NOT stored on disk. After loading, all
devices start as offline; the inventory task sets them online after a successful CoAP
`/state` poll or after `/discover` response.

### 6.3 Flash Survival

The `iris_devices.bin` file lives on the SPIFFS partition. `idf.py flash` overwrites SPIFFS.
Use `idf.py app-flash` during development to preserve paired device data.

After an accidental full flash, the discovery sweep (section 3.3) automatically
restores all devices that are still in the Thread network — no manual re-pairing needed.

### 6.4 Integrity

On read: verify `magic == 0x49524953`. If mismatch (e.g., partial write), treat as empty device list and log an error.

---

## 7. C6 Iris API Reference

Key public functions in `components/iris/include/iris/iris.h`:

| Function | Description |
|----------|-------------|
| `iris_init()` | Init OpenThread, load SPIFFS, register neighbor callback |
| `iris_start_inventory_task()` | Start background poll + run initial discovery |
| `iris_run_discovery()` | Blocking multicast sweep (call from task context) |
| `iris_scan(out, max)` | Get list of new joiners (Commissioner cache) |
| `iris_pair(eui64, name)` | Provision a new joiner into paired list |
| `iris_get_paired(out, max)` | Get all paired devices |
| `iris_toggle(eui64, cap)` | Unicast toggle one capability |
| `iris_set_all(cap, on)` | Multicast explicit state set |
| `iris_unpair(eui64)` | Remove from paired list + SPIFFS |
| `iris_any_has_cap(cap)` | Check if any paired device has a capability |
| `iris_is_master()` | Returns true if this unit is active Master |

### Kconfig parameters (`components/iris/Kconfig`)

| Key | Default | Description |
|-----|---------|-------------|
| `IRIS_MAX_DEVICES` | 32 | Max paired devices (up to 64) |
| `IRIS_INVENTORY_INTERVAL_MS` | 30000 | Poll interval per device |
| `IRIS_OFFLINE_THRESHOLD` | 3 | Failed polls before marking offline |
| `IRIS_DISCOVERY_WINDOW_MS` | 3000 | How long to collect `/discover` responses |
| `IRIS_DISCOVERY_INTERVAL_CYCLES` | 10 | Full discovery every N poll cycles (≈5 min) |
| `IRIS_JOINER_PSKD` | `"JOINPW01"` | PSKd shared with H2 firmware |
| `IRIS_MASTER_PRIORITY` | 1 | Election priority (Primary C6: set to 2) |
| `IRIS_MASTER_HEARTBEAT_INTERVAL_MS` | 5000 | Master heartbeat interval |
| `IRIS_MASTER_FAILOVER_TIMEOUT_MS` | 15000 | Standby failover trigger timeout |

---

## 8. H2 Quickstart (Implementation Reference)

Minimal ESP32-H2 firmware skeleton. Adapt to your project structure.

### 8.1 Thread Stack Init + Join

```c
#include "esp_openthread.h"
#include "openthread/joiner.h"
#include "openthread/coap.h"
#include "openthread/instance.h"

#define JOINER_PSKD   "JOINPW01"   /* Must match CONFIG_IRIS_JOINER_PSKD on C6 */

static otInstance *s_instance;

static void joiner_callback(otError error, void *ctx) {
    if (error == OT_ERROR_NONE) {
        ESP_LOGI("H2", "Thread joined successfully");
        otCoapStart(s_instance, OT_DEFAULT_COAP_PORT);
        coap_register_resources(s_instance);
    } else {
        ESP_LOGE("H2", "Thread join failed: %d", error);
        // Retry after delay
    }
}

void thread_init(void) {
    esp_openthread_platform_config_t config = {
        .radio_config = { .radio_mode = RADIO_MODE_NATIVE },
        .host_config  = { .host_connection_mode = HOST_CONNECTION_MODE_NONE },
        .port_config  = { .storage_partition_name = "nvs",
                          .netif_queue_size = 10, .task_queue_size = 10 },
    };
    esp_openthread_init(&config);
    s_instance = esp_openthread_get_instance();

    otJoinerStart(s_instance, JOINER_PSKD, NULL,
                  "MyVendor", "ModelRailH2", "1.0",
                  NULL, joiner_callback, NULL);

    // Blocks — run in a dedicated FreeRTOS task
    esp_openthread_launch_mainloop();
}
```

### 8.2 CoAP Server Resources

```c
/* Device capabilities — set based on hardware */
#define MY_CAPS  (IRIS_CAP_INNER_LIGHT | IRIS_CAP_MOVEMENT)
static uint8_t s_state = 0;

/* Optional: persist name in NVS so /discover can return it */
static const char *MY_NAME = "Wagen 01";

static void handle_capabilities(void *ctx, otMessage *msg,
                                 const otMessageInfo *info) {
    char buf[32];
    snprintf(buf, sizeof(buf), "{\"caps\":%u}", (unsigned)MY_CAPS);
    // ... send CoAP response with buf ...
}

static void handle_state(void *ctx, otMessage *msg,
                          const otMessageInfo *info) {
    char buf[32];
    snprintf(buf, sizeof(buf), "{\"state\":%u}", (unsigned)s_state);
    // ... send CoAP response with buf ...
}

/* GET /discover — multicast discovery response */
static void handle_discover(void *ctx, otMessage *msg,
                             const otMessageInfo *info) {
    otExtAddress eui64;
    otLinkGetExtendedAddress(s_instance, &eui64);

    char eui_str[17];
    for (int i = 0; i < 8; i++)
        snprintf(eui_str + i * 2, 3, "%02x", eui64.m8[i]);

    char buf[128];
    snprintf(buf, sizeof(buf),
             "{\"eui64\":\"%s\",\"caps\":%u,\"state\":%u,\"name\":\"%s\"}",
             eui_str, (unsigned)MY_CAPS, (unsigned)s_state, MY_NAME);
    // ... send CoAP response with buf ...
}

/* POST /toggle — unicast toggle from OLED menu */
static void handle_toggle(void *ctx, otMessage *msg,
                           const otMessageInfo *info) {
    char buf[64] = {};
    uint16_t len = otMessageGetLength(msg) - otMessageGetOffset(msg);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    otMessageRead(msg, otMessageGetOffset(msg), buf, len);

    unsigned cap = 0;
    sscanf(buf, "{\"cap\":%u}", &cap);

    if (cap & MY_CAPS) {
        s_state ^= (uint8_t)cap;  // toggle the bit
        apply_state(s_state);
    }
    // ... send 2.04 Changed ...
}

/* POST /set — multicast explicit state ("Alle Innen AN/AUS") */
static void handle_set(void *ctx, otMessage *msg,
                        const otMessageInfo *info) {
    char buf[64] = {};
    uint16_t len = otMessageGetLength(msg) - otMessageGetOffset(msg);
    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    otMessageRead(msg, otMessageGetOffset(msg), buf, len);

    unsigned cap = 0, state = 0;
    sscanf(buf, "{\"cap\":%u,\"state\":%u}", &cap, &state);

    if (cap & MY_CAPS) {
        // Apply explicit state — do NOT toggle
        if (state)
            s_state |= (uint8_t)cap;
        else
            s_state &= (uint8_t)~cap;
        apply_state(s_state);
    }
    // NON request — no response needed
}

static otCoapResource s_res_caps     = {"capabilities", handle_capabilities, NULL, NULL};
static otCoapResource s_res_state    = {"state",        handle_state,        NULL, NULL};
static otCoapResource s_res_discover = {"discover",     handle_discover,     NULL, NULL};
static otCoapResource s_res_toggle   = {"toggle",       handle_toggle,       NULL, NULL};
static otCoapResource s_res_set      = {"set",          handle_set,          NULL, NULL};

void coap_register_resources(otInstance *inst) {
    otCoapAddResource(inst, &s_res_caps);
    otCoapAddResource(inst, &s_res_state);
    otCoapAddResource(inst, &s_res_discover);
    otCoapAddResource(inst, &s_res_toggle);
    otCoapAddResource(inst, &s_res_set);
}
```

### 8.3 Multicast Subscription

To receive multicast commands and discovery requests on `ff03::1`:

```c
otIp6Address multicast_addr;
otIp6AddressFromString("ff03::1", &multicast_addr);
otIp6SubscribeMulticastAddress(s_instance, &multicast_addr);
```

This is typically handled automatically by the Thread stack for realm-local scope, but
explicit subscription ensures the CoAP server receives these datagrams.

---

## 9. Build Environment (C6 Firmware)

### 9.1 Required sdkconfig settings

The following must be present in `sdkconfig.defaults.esp32c6` (already set):

```
CONFIG_OPENTHREAD_ENABLED=y
CONFIG_OPENTHREAD_BORDER_ROUTER=y
CONFIG_OPENTHREAD_COMMISSIONER=y
CONFIG_OPENTHREAD_RADIO_NATIVE=y
CONFIG_LWIP_IPV6=y
CONFIG_LWIP_IPV6_NUM_ADDRESSES=12   # must be 12 (OpenThread requirement)
CONFIG_MBEDTLS_SSL_PROTO_DTLS=y     # DTLS required for Commissioner/Joiner
CONFIG_MBEDTLS_KEY_EXCHANGE_ECJPAKE=y
CONFIG_MBEDTLS_ECJPAKE_C=y
CONFIG_IRIS_ENABLED=y
```

### 9.2 Build commands

```bash
# Initialize ESP-IDF 5.5.3
. /Users/mars3142/.espressif/v5.5.3/esp-idf/export.sh

# Build for ESP32-C6
cd /Users/mars3142/Coding/git.mars3142.dev/system-control/firmware
idf.py -DIDF_TARGET=esp32c6 build

# Flash only the app (preserves SPIFFS / paired device list)
idf.py -p <PORT> app-flash

# Flash everything including SPIFFS (paired device list will be auto-restored
# on next boot via discovery sweep — see section 3.3)
idf.py -p <PORT> flash
```

If `export.sh` cannot find the Python environment, invoke `idf.py` directly:
```bash
IDF_PYTHON_ENV_PATH=/Users/mars3142/.espressif/tools/python/v5.5.3/venv \
IDF_TOOLS_PATH=/Users/mars3142/.espressif/tools \
/Users/mars3142/.espressif/tools/python/v5.5.3/venv/bin/python \
  /Users/mars3142/.espressif/v5.5.3/esp-idf/tools/idf.py -DIDF_TARGET=esp32c6 build
```

### 9.3 Source layout

```
components/iris/
  include/iris/
    iris.h              ← public API
    iris_internal.h     ← private shared state (extern variables + internal declarations)
  src/
    iris.c              ← state definitions, public API impl, stubs
    iris_storage.c      ← spiffs_save / spiffs_load
    iris_coap.c         ← eui64_to_ml_eid, coap_get, coap_post
    iris_discovery.c    ← neighbor callback, joiner callback, /discover, iris_run_discovery
    iris_master.c       ← master election, heartbeat, iris_master_task
    iris_inventory.c    ← iris_inventory_task
  Kconfig
  CMakeLists.txt
```
