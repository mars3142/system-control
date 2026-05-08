# Thread Integration — ESP32-C6 Coordinator

This document describes how the ESP32-C6 system-control unit forms and manages a
Thread network, discovers and controls ESP32-H2 devices (lighthouses), and exposes
the full device lifecycle via REST API and WebSocket.

For the **H2 device perspective** (Joiner, CoAP server, Observe server) see
`README-THREAD.md` in the `warnemuende-lighthouses/firmware` repository.

---

## 1 — Architecture

```
ESP32-C6  (system-control)
├── WiFi  ──────────────────── Web UI / REST API / WebSocket
└── Thread / IEEE 802.15.4
    ├── Role: Leader (preferred) or Router
    ├── Commissioner: accepts Joiners with PSKd MAERKLN
    ├── CoAP server: listens for /announce
    └── CoAP client: controls H2 devices via unicast + multicast
            │
            └── ESP32-H2 (Joiner / Full Thread Device)
                    CoAP server: /beacon  /outdoor  /group
                    CoAP Observe server: /beacon  /outdoor  (RFC 7641)
```

The C6 is the **only** Commissioner on the network. H2 devices never form a network
themselves — they always join as Children or Routers with minimal leader priority.

---

## 2 — Thread network formation

On first boot (no stored dataset) the C6 calls `otDatasetCreateNewNetwork` to create
a random Thread network with a fresh Mesh-Local Prefix. The dataset is stored inside
OpenThread's own NVS partition and survives reboots.

On subsequent boots the C6 resumes the same network (`otDatasetIsCommissioned`
returns true). The Mesh-Local Prefix therefore remains **the same across reboots**,
which keeps all H2 device addresses stable (see section 7).

The C6 tries to become **Leader** (highest router priority). Once it holds the Leader
role it starts the Commissioner automatically.

---

## 3 — Commissioner

The C6 starts a wildcard Joiner entry as soon as it becomes Leader:

```c
otCommissionerStart(instance, NULL, NULL, NULL);
otCommissionerAddJoiner(instance, NULL, "MAERKLN", 120);
//                       ^^^^ NULL = any EUI-64
```

| Parameter | Value                                |
| --------- | ------------------------------------ |
| PSKd      | `MAERKLN`                            |
| Timeout   | 120 s (restarted on every role gain) |
| Vendor    | Maerklin / Lighthouse / 1.0          |

A new H2 that has no stored dataset starts the Joiner role on boot, discovers the
Commissioner via CoAP, performs EC-JPAKE (DTLS + mbedTLS J-PAKE), receives the
dataset and joins. After that, the H2 keeps its dataset through reboots and never
needs to re-commission.

---

## 4 — Device discovery

After joining the network the H2 sends a single **NON-CONFIRMABLE CoAP POST**:

```
Destination : coap://[ff03::1]:5683/announce
Payload     : <device-name>    e.g. "Leuchtturm West"
```

The C6's CoAP server listens on `/announce` and records:

- **Source IPv6 address** → used for all subsequent unicast CoAP
- **Device name** → stored in NVS, shown in API and web UI

If the C6 restarts, it must wait for the H2 to re-announce (which happens on H2
reboot) or the user can manually add a device via `POST /api/thread/devices`.

---

## 5 — Capability discovery

Immediately after recording an announce, the C6 sends:

```
GET coap://[<H2-addr>]:5683/.well-known/core
```

Expected response (RFC 6690 link-format):

```
</beacon>;rt="maerklin.switch";title="Beacon";obs,
</outdoor>;rt="maerklin.switch";title="Outdoor";obs,
</group>;rt="maerklin.group";title="Group"
```

The C6 parses this to set `has_beacon` and `has_outdoor` on the device entry.
The `obs` attribute (RFC 7641) signals that the resource supports CoAP Observe —
the C6 immediately registers as an observer on each `obs`-flagged resource
(see section 6).

Capabilities are **persisted in NVS** so the web UI knows which controls to show
even after a C6 reboot (before the H2 re-announces).

---

## 6 — CoAP Observe (RFC 7641)

After capabilities are discovered, the C6 registers as a CoAP observer on every
resource that carries the `obs` attribute (`/beacon` and `/outdoor`).

### Why

- CoAP Observe gives the C6 real-time state without polling.
- After a **multicast group command** (which cannot carry individual ACKs), each
  device sends a CON Observe notification back to the C6. The C6's ACK is proof of
  delivery — a device that does not send a notification (or does not ACK the C6's
  cancel) failed to execute the command.

### Registration

The C6 sends a **CONFIRMABLE GET** with the `Observe: 0` option:

```
GET coap://[<H2-addr>]:5683/beacon
Observe: 0
Token: <8-byte random token>
```

The H2 responds with the current resource state and an Observe sequence number.
The same callback receives all subsequent **CON 2.05 Content** notifications from
the H2:

```c
// observe_notification_cb — called for initial response AND every notification
if (result != OT_ERROR_NONE) {
    // mark device unreachable (only on first error to avoid duplicate events)
    if (dev->reachable) {
        dev->reachable = false;
        emit("{\"type\":\"thread_unreachable\",\"addr\":\"...\"}");
    }
    free(ctx);    // observation ended (timeout, RST, cancel)
    return;
}
bool is_on = (payload[0] == '1');
// → update device state, emit "thread_state" WebSocket event
```

OpenThread ACKs incoming CON messages automatically.

### Cancellation

When a device is **removed via the API** while an observation is active, the next
incoming notification triggers an automatic deregister:

```c
send_observe_cancel(instance, notification_msg, msg_info, resource);
// sends: GET /beacon  Observe: 1  Token: <same as registration>
free(ctx);
```

This frees the observer slot on the H2 (`MAX_OBSERVERS = 4` per resource).

### Re-registration after C6 reboot

On reboot, the C6 loads all devices from NVS. When it becomes Leader or Router
(`state_changed_cb`), `register_all_observers` re-sends Observe registrations for
every device that has known capabilities. The initial response brings the current
hardware state.

### Notification flow

```
C6 reboots
 └─ loads devices from NVS
 └─ becomes Leader
      └─ register_all_observers()
            └─ GET /beacon Observe:0  ──►  H2
               ◄──────────────────────  2.05 Content (current state)
                                        observe_notification_cb fires
                                        → beacon_on = true/false
                                        → WS event "thread_state"

H2 changes state (e.g. user presses button on H2)
 └─ H2 sends CON 2.05 to all observers
      └─ observe_notification_cb fires
            → beacon_on updated
            → WS event "thread_state"
```

---

## 7 — Reachability monitoring

The C6 tracks each device's reachability without polling or dedicated ping commands.

### Detecting unreachable

When a device goes offline, incoming CoAP Observe notifications stop. OpenThread
eventually calls `observe_notification_cb` with `result != OT_ERROR_NONE` (timeout
or RST). At that point:

1. `dev->reachable` is set to `false`
2. A `thread_unreachable` WebSocket event is emitted (only on the first error —
   devices with both `beacon` and `outdoor` observers do not emit duplicates)
3. The dead observe context is freed

The detection latency depends on the CoAP retransmit timeout (~45 s by default).

### Detecting reachable again

The C6 registers a `otThreadRegisterNeighborTableCallback`. When any neighbor
joins the Thread network (`OT_NEIGHBOR_TABLE_EVENT_CHILD_ADDED` or
`ROUTER_ADDED`), the callback:

1. Snapshots all devices with `reachable == false`
2. Sends a `.well-known/core` CoAP query to each of them

If a device responds, `caps_response_cb` fires:

- `dev->reachable` is set to `true`
- CoAP Observe is re-registered for `beacon` and `outdoor`
- A `thread_capabilities` WebSocket event is emitted

The client therefore uses `thread_unreachable` as the "gone" signal and
`thread_capabilities` as the "back" signal.

### Scenario overview

| Event | Mechanism | WS event |
| ----- | --------- | -------- |
| Device powers off / RF loss | Observe timeout → `observe_notification_cb` error | `thread_unreachable` |
| Device reboots | H2 sends `/announce` → caps query | `thread_capabilities` |
| Device rejoins without reboot | Neighbor table `ADDED` → caps query to all unreachable | `thread_capabilities` |
| C6 reboots, device was already in mesh | `register_all_observers()` on Leader role → initial Observe response | `thread_state` (not `thread_capabilities`) |

---

## 8 — IPv6 address stability

The C6 stores each H2's **Mesh-Local EID** (e.g. `fd12:3456:789a::eui64`).
This address has two components:

```
fd12:3456:789a:0000  :  XX:XX:XX:ff:fe:XX:XX:XX
└── Mesh-Local Prefix ┘  └── IID from EUI-64 ────┘
    from Thread dataset       hardware identifier
```

| Component | Origin | Changes? |
| --------- | ------ | -------- |
| Mesh-Local Prefix | Thread dataset on C6 | Only on factory reset (new network) |
| Interface Identifier | H2's EUI-64 (802.15.4 chip, fixed) | Never |

**Conclusion:** under normal operation (reboots of C6 or H2 without factory reset)
the address is **the same every time**. The NVS device list remains valid across
power cycles.

The only scenario that invalidates stored addresses is a **C6 factory reset** (new
Thread network → new Mesh-Local Prefix). Since `persistence_manager_factory_reset()`
erases all NVS, the device list is wiped at the same time — the C6 never holds stale
addresses.

> Note: the **RLOC** (`fd..::ff:fe00:xxxx`) changes after every reboot and is not
> used for application-level CoAP communication.

---

## 9 — Device control

All unicast control uses **NON-CONFIRMABLE CoAP PUT** (fire-and-forget). The HTTP
handler acquires the OpenThread API lock (`esp_openthread_lock_acquire`) to call OT
APIs directly, then releases it immediately after `otCoapSendRequest`:

```
PUT coap://[<H2-addr>]:5683/beacon
Payload: "1"   (on)
         "0"   (off)
```

```
PUT coap://[<H2-addr>]:5683/outdoor
Payload: "1" / "0"
```

REST: `POST /api/thread/devices/set`

---

## 10 — Group management

Groups use **IPv6 multicast** in the mesh-local scope `ff03::/16`. The C6 manages
group membership by telling each H2 which multicast address to subscribe to.

### Joining a group

```
PUT coap://[<H2-addr>]:5683/group
Payload: "ff03::10,1"    ← <multicast-addr>,1
```

### Leaving a group

```
PUT coap://[<H2-addr>]:5683/group
Payload: "ff03::10,0"    ← <multicast-addr>,0
```

### Sending a group command

```
PUT coap://[ff03::10]:5683/outdoor
Payload: "1"
```

All H2 devices subscribed to `ff03::10` receive this simultaneously.
After switching, each device sends an individual Observe notification back to
the C6 — the C6 collects these as confirmation that every member executed the
command.

### Recommended multicast addresses

| Address    | Suggested use           |
| ---------- | ----------------------- |
| `ff03::10` | All outdoor lamps       |
| `ff03::11` | All beacons             |
| `ff03::20` | Scene: harbour lights   |

### Persistence

Groups (name, multicast address, member list) are stored in NVS namespace
`thread_mgr` key `groups`. Group membership on the H2 is also persisted on the H2
itself — after H2 reboot the device rejoins its multicast subscriptions automatically.

On `DELETE /api/thread/groups` the C6 sends CoAP group-leave to every member before
deleting the local record. On `DELETE /api/thread/devices` the C6 removes the device
from all groups first.

---

## 11 — NVS layout

All Thread-manager state lives in the `thread_mgr` namespace:

| Key       | Type | Content |
| --------- | ---- | ------- |
| `devices` | blob | `thread_device_t[]` — name, IPv6, has\_beacon, has\_outdoor |
| `groups`  | blob | `thread_group_t[]` — name, multicast addr, member addr list |

`reachable`, `beacon_on`, `outdoor_on` are runtime-only and not persisted
(refreshed via Observe on next network join).

Blob size encodes the element count: `count = blob_size / sizeof(element)`.

---

## 12 — Limits

| Parameter | Value | Defined in |
| --------- | ----- | ---------- |
| Max tracked devices | 16 | `THREAD_MAX_DEVICES` |
| Max groups | 8 | `THREAD_MAX_GROUPS` |
| Max devices per group | 8 | `THREAD_GROUP_MAX_MEMBERS` |
| Max Observe observers per resource (H2-side) | 4 | `MAX_OBSERVERS` in lighthouse firmware |
| Commissioner timeout | 120 s | `COMMISSIONER_TIMEOUT_S` |
| Joiner PSKd | `MAERKLN` | `COMMISSIONER_PSK` |

---

## 13 — REST API quick reference

| Method | URL | Action |
| ------ | --- | ------ |
| `GET` | `/api/thread/devices` | List all devices (name, addr, state) |
| `POST` | `/api/thread/devices` | Add device manually `{name, addr}` |
| `DELETE` | `/api/thread/devices` | Remove device `{addr}` |
| `POST` | `/api/thread/devices/set` | Control resource `{addr, resource, on}` |
| `GET` | `/api/thread/groups` | List all groups with members |
| `POST` | `/api/thread/groups` | Create group `{name, addr}` |
| `DELETE` | `/api/thread/groups` | Delete group `{addr}` |
| `POST` | `/api/thread/groups/assign` | Add device to group `{device_addr, group_addr}` |
| `DELETE` | `/api/thread/groups/assign` | Remove device from group |
| `POST` | `/api/thread/groups/command` | Group command `{addr, resource, on}` |

Full request/response schemas: `README-API.md` → sections *Thread Devices* and
*Thread Groups*.

---

## 14 — WebSocket events

| Type | Trigger |
| ---- | ------- |
| `thread_device` | H2 announced (or manually added) |
| `thread_device_removed` | Device deleted via API |
| `thread_capabilities` | `/.well-known/core` query completed — also signals device is **reachable again** |
| `thread_state` | CoAP Observe notification received (RFC 7641) |
| `thread_unreachable` | CoAP Observe timed out / device went offline |
| `thread_group_added` | Group created |
| `thread_group_removed` | Group deleted |
| `thread_group_assigned` | Device added to group |
| `thread_group_unassigned` | Device removed from group |

Full event schemas: `README-API.md` → section *WebSocket*.

---

## 15 — Lifecycle summary

```
C6 boot
 ├─ load devices + groups from NVS
 ├─ start Thread stack (thread_task)
 │   ├─ resume existing dataset (or create new network)
 │   ├─ become Leader → start Commissioner (PSKd MAERKLN)
 │   └─ register_all_observers() → GET /beacon + /outdoor for all known devices
 │
 ├─ H2 joins (commissioning via EC-JPAKE)
 │   └─ H2 sends announce → C6 records addr + name
 │       └─ C6 queries /.well-known/core
 │           └─ capabilities stored → Observe registered for /beacon + /outdoor
 │
 ├─ H2 state changes
 │   └─ H2 sends CON Observe notification → C6 ACKs → updates beacon_on/outdoor_on
 │       └─ WS event "thread_state" broadcast to web clients
 │
 ├─ H2 goes offline (power loss / RF)
 │   └─ Observe notifications stop → timeout → observe_notification_cb error
 │       └─ reachable = false → WS event "thread_unreachable"
 │
 ├─ H2 comes back (reboot)
 │   └─ H2 sends /announce → caps query → reachable = true + Observe re-registered
 │       └─ WS event "thread_capabilities"
 │
 ├─ H2 comes back (rejoin without reboot)
 │   └─ neighbor_table_cb CHILD_ADDED → caps query to all unreachable devices
 │       └─ H2 responds → reachable = true + Observe re-registered
 │           └─ WS event "thread_capabilities"
 │
 ├─ Web client: POST /api/thread/devices/set
 │   └─ NON-CONF CoAP PUT (OT lock held) → H2 → switches → Observe notification → WS event
 │
 ├─ Web client: POST /api/thread/groups/command
 │   └─ NON-CONF CoAP PUT to ff03::10 → all members switch
 │       └─ each member sends individual Observe notification → WS events
 │
 └─ C6 factory reset
     ├─ persistence_manager_factory_reset() → NVS erased (devices + groups cleared)
     └─ Thread dataset erased → new network on next boot → new Mesh-Local Prefix
```
