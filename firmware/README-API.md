# System Control - API Documentation

This document describes all REST API endpoints and WebSocket messages required for the ESP32 firmware implementation.

## Table of Contents

- [REST API Endpoints](#rest-api-endpoints)
  - [Capabilities](#capabilities)
  - [WiFi](#wifi)
  - [Light Control](#light-control)
  - [LED Configuration](#led-configuration)
  - [Schema](#schema)
  - [Thread Devices](#thread-devices)
  - [Thread Groups](#thread-groups)
  - [Scenes](#scenes)
  - [Input](#input)
- [WebSocket](#websocket)
  - [Connection](#connection)
  - [Client to Server Messages](#client-to-server-messages)
  - [Server to Client Messages](#server-to-client-messages)
    - [Thread: Resource State Change (RFC 7641 Observe)](#thread-resource-state-change-rfc-7641-observe)

---

## REST API Endpoints

### Capabilities

#### Get Device Capabilities

Returns the device capabilities. Used to determine which features are available.

- **URL:** `/api/capabilities`
- **Method:** `GET`
- **Response:**

```json
{
  "thread": true
}
```

| Field  | Type    | Description                                           |
|--------|---------|-------------------------------------------------------|
| thread | boolean | Whether Thread/Matter features are enabled            |

**Notes:**
- If `thread` is `true`, the UI shows Matter device management and Scenes
- If `thread` is `false` or the endpoint is unavailable, these features are hidden
- The client can also force-enable features via URL parameter `?thread=true`

---

### WiFi

#### Scan Networks

Scans for available WiFi networks.

- **URL:** `/api/wifi/scan`
- **Method:** `GET`
- **Response:**

```json
[
  {
    "ssid": "NetworkName",
    "rssi": -45
  },
  {
    "ssid": "AnotherNetwork",
    "rssi": -72
  }
]
```

| Field | Type   | Description                        |
|-------|--------|------------------------------------|
| ssid  | string | Network name (SSID)                |
| rssi  | number | Signal strength in dBm             |

---

#### Save WiFi Configuration

Saves WiFi credentials and initiates connection.

- **URL:** `/api/wifi/config`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "ssid": "NetworkName",
  "password": "SecretPassword123"
}
```

| Field    | Type   | Required | Description       |
|----------|--------|----------|-------------------|
| ssid     | string | Yes      | Network name      |
| password | string | No       | Network password  |

- **Response:** `200 OK` on success, `400 Bad Request` on error

---

#### Get WiFi Status

Returns current WiFi connection status.

- **URL:** `/api/wifi/status`
- **Method:** `GET`
- **Response:**

```json
{
  "connected": true,
  "ssid": "NetworkName",
  "ip": "192.168.1.100",
  "rssi": -45
}
```

| Field     | Type    | Description                              |
|-----------|---------|------------------------------------------|
| connected | boolean | Whether connected to a network           |
| ssid      | string  | Connected network name (if connected)    |
| ip        | string  | Assigned IP address (if connected)       |
| rssi      | number  | Signal strength in dBm (if connected)    |

---

### Light Control

#### Set Light Power

Turns the main light on or off.

- **URL:** `/api/light/power`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "on": true
}
```

| Field | Type    | Required | Description              |
|-------|---------|----------|--------------------------|
| on    | boolean | Yes      | `true` = on, `false` = off |

- **Response:** `200 OK` on success

---

#### Set Thunder Effect

Turns the thunder/lightning effect on or off.

- **URL:** `/api/light/thunder`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "on": true
}
```

| Field | Type    | Required | Description                    |
|-------|---------|----------|--------------------------------|
| on    | boolean | Yes      | `true` = on, `false` = off     |

- **Response:** `200 OK` on success

**Notes:**
- When enabled, random lightning flashes are triggered
- Can be combined with any light mode
- Thunder effect stops automatically when light is turned off

---

#### Set Light Mode

Sets the lighting mode.

- **URL:** `/api/light/mode`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "mode": "simulation"
}
```

| Field | Type   | Required | Description                                    |
|-------|--------|----------|------------------------------------------------|
| mode  | string | Yes      | One of: `day`, `night`, `simulation`           |

- **Response:** `200 OK` on success

---

#### Set Active Schema

Sets the active schema for simulation mode.

- **URL:** `/api/light/schema`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "schema": "schema_01.csv"
}
```

| Field  | Type   | Required | Description                                           |
|--------|--------|----------|-------------------------------------------------------|
| schema | string | Yes      | Schema filename: `schema_01.csv`, `schema_02.csv`, etc. |

- **Response:** `200 OK` on success

---

#### Get Light Status

Returns current light status (alternative to WebSocket).

- **URL:** `/api/light/status`
- **Method:** `GET`
- **Response:**

```json
{
  "on": true,
  "thunder": false,
  "mode": "simulation",
  "schema": "schema_01.csv",
  "color": {
    "r": 255,
    "g": 240,
    "b": 220
  }
}
```

| Field   | Type    | Description                          |
|---------|---------|--------------------------------------|
| on      | boolean | Current power state                  |
| thunder | boolean | Current thunder effect state         |
| mode    | string  | Current mode (day/night/simulation)  |
| schema  | string  | Active schema filename               |
| color   | object  | Current RGB color being displayed    |

---

### LED Configuration

#### Get LED Configuration

Returns the current LED segment configuration.

- **URL:** `/api/wled/config`
- **Method:** `GET`
- **Response:**

```json
{
  "segments": [
    {
      "name": "Main Light",
      "start": 0,
      "leds": 60
    },
    {
      "name": "Accent Light",
      "start": 60,
      "leds": 30
    }
  ]
}
```

| Field              | Type   | Description                              |
|--------------------|--------|------------------------------------------|
| segments           | array  | List of LED segments                     |
| segments[].name    | string | Optional segment name                    |
| segments[].start   | number | Start LED index (0-based)                |
| segments[].leds    | number | Number of LEDs in this segment           |

---

#### Save LED Configuration

Saves the LED segment configuration.

- **URL:** `/api/wled/config`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "segments": [
    {
      "name": "Main Light",
      "start": 0,
      "leds": 60
    },
    {
      "name": "Accent Light",
      "start": 60,
      "leds": 30
    }
  ]
}
```

| Field              | Type   | Required | Description                              |
|--------------------|--------|----------|------------------------------------------|
| segments           | array  | Yes      | List of LED segments (can be empty)      |
| segments[].name    | string | No       | Optional segment name                    |
| segments[].start   | number | Yes      | Start LED index (0-based)                |
| segments[].leds    | number | Yes      | Number of LEDs in this segment           |

- **Response:** `200 OK` on success, `400 Bad Request` on validation error

**Notes:**
- Segments define how the LED strip is divided into logical groups
- Changes are persisted to NVS (non-volatile storage)
- Each segment can be controlled independently in the light schema

---

### Schema

#### Load Schema

Loads a schema file.

- **URL:** `/api/schema/{filename}`
- **Method:** `GET`
- **URL Parameters:**
  - `filename`: Schema file name (e.g., `schema_01.csv`)
- **Response:** CSV text data

```
255,240,220,0,100,250
255,230,200,0,120,250
...
```

The CSV format has 48 rows (one per 30-minute interval) with 6 values per row:

| Column | Description                    | Range   |
|--------|--------------------------------|---------|
| 1      | Red (R)                        | 0-255   |
| 2      | Green (G)                      | 0-255   |
| 3      | Blue (B)                       | 0-255   |
| 4      | Value 1 (V1) - custom value    | 0-255   |
| 5      | Value 2 (V2) - custom value    | 0-255   |
| 6      | Value 3 (V3) - custom value    | 0-255   |

---

#### Save Schema

Saves a schema file.

- **URL:** `/api/schema/{filename}`
- **Method:** `POST`
- **Content-Type:** `text/csv`
- **URL Parameters:**
  - `filename`: Schema file name (e.g., `schema_01.csv`)
- **Request Body:** CSV text data (same format as above)
- **Response:** `200 OK` on success

---

### Thread Devices

Manages OpenThread devices (e.g. ESP32-H2 lighthouses). Devices join the Thread network automatically and announce themselves via CoAP. They can also be added manually by IPv6 address.

---

#### List Thread Devices

Returns all known Thread devices (auto-discovered via CoAP announce + manually added). Persisted in NVS.

- **URL:** `/api/thread/devices`
- **Method:** `GET`
- **Response:**

```json
[
  {
    "name": "Leuchtturm West",
    "addr": "fd12:3456:789a::1",
    "has_beacon": true,
    "has_outdoor": true,
    "reachable": true,
    "beacon_on": false,
    "outdoor_on": true
  }
]
```

| Field       | Type    | Description                                                                    |
|-------------|---------|--------------------------------------------------------------------------------|
| name        | string  | Device name (from announce or manually set)                                    |
| addr        | string  | Mesh-local IPv6 address                                                        |
| has_beacon  | boolean | Device exposes `/beacon` CoAP resource                                         |
| has_outdoor | boolean | Device exposes `/outdoor` CoAP resource                                        |
| reachable   | boolean | True after last successful CoAP response (resets on boot)                      |
| beacon_on   | boolean | Current beacon state — updated via CoAP Observe (RFC 7641), `false` until first notification |
| outdoor_on  | boolean | Current outdoor lamp state — updated via CoAP Observe (RFC 7641), `false` until first notification |

**Notes:**
- After the C6 registers as a CoAP observer (`GET /beacon` with `Observe: 0`), the H2 pushes a `thread_state` WebSocket event on every state change — no polling needed.
- `beacon_on` / `outdoor_on` start as `false` on boot and are set to the actual device state on the first observer notification, which arrives within seconds of the Observe registration.
- Observe registrations survive normal device reboots: when the H2 re-announces after a reboot, the C6 re-registers automatically.

---

#### Add Thread Device

Manually registers a Thread device by name and IPv6 address. Capabilities are discovered asynchronously via CoAP and arrive as a `thread_capabilities` WebSocket event. Idempotent: if `addr` already exists, the name is updated.

- **URL:** `/api/thread/devices`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "name": "Leuchtturm Ost",
  "addr": "fd12:3456:789a::2"
}
```

| Field | Type   | Required | Description             |
|-------|--------|----------|-------------------------|
| name  | string | Yes      | Human-readable label    |
| addr  | string | Yes      | Mesh-local IPv6 address |

- **Response:** `{"ok":true}` on success, `507` if device limit (16) reached

---

#### Remove Thread Device

Removes a device from NVS. The device is also removed from all groups and CoAP group-leave messages are sent.

- **URL:** `/api/thread/devices`
- **Method:** `DELETE`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "addr": "fd12:3456:789a::2"
}
```

| Field | Type   | Required | Description             |
|-------|--------|----------|-------------------------|
| addr  | string | Yes      | Mesh-local IPv6 address |

- **Response:** `{"ok":true}` on success, `404` if not found

---

#### Control Device Resource

Sends a CoAP PUT to a device resource. Fire-and-forget.

- **URL:** `/api/thread/devices/set`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "addr": "fd12:3456:789a::1",
  "resource": "beacon",
  "on": true
}
```

| Field    | Type    | Required | Description                          |
|----------|---------|----------|--------------------------------------|
| addr     | string  | Yes      | Target device IPv6 address           |
| resource | string  | Yes      | `"beacon"` or `"outdoor"`            |
| on       | boolean | Yes      | `true` = on, `false` = off           |

- **Response:** `{"ok":true}` on success

---

### Thread Groups

Groups use IPv6 multicast addresses (`ff03::/16` mesh-local scope). Devices subscribed to a group respond to CoAP commands sent to the multicast address.

Suggested multicast addresses:
- `ff03::10` — All outdoor lamps
- `ff03::11` — All beacons
- `ff03::20` — All devices (harbour scene)

---

#### List Groups

Returns all groups with their members.

- **URL:** `/api/thread/groups`
- **Method:** `GET`
- **Response:**

```json
[
  {
    "name": "Alle Aussenlampen",
    "addr": "ff03::10",
    "members": ["fd12:3456:789a::1", "fd12:3456:789a::2"]
  }
]
```

| Field   | Type   | Description                              |
|---------|--------|------------------------------------------|
| name    | string | Group label                              |
| addr    | string | IPv6 multicast address                   |
| members | array  | List of member device IPv6 addresses     |

---

#### Create Group

Creates a new group. The multicast address must be unique.

- **URL:** `/api/thread/groups`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "name": "Alle Aussenlampen",
  "addr": "ff03::10"
}
```

| Field | Type   | Required | Description             |
|-------|--------|----------|-------------------------|
| name  | string | Yes      | Group label             |
| addr  | string | Yes      | IPv6 multicast address  |

- **Response:** `{"ok":true}` on success, `409` if address already in use, `507` if group limit (8) reached

---

#### Delete Group

Removes a group. CoAP group-leave messages are sent to all current members.

- **URL:** `/api/thread/groups`
- **Method:** `DELETE`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "addr": "ff03::10"
}
```

| Field | Type   | Required | Description            |
|-------|--------|----------|------------------------|
| addr  | string | Yes      | Group multicast address |

- **Response:** `{"ok":true}` on success, `404` if not found

---

#### Assign Device to Group

Adds a device to a group. The device receives a CoAP PUT `/group` with payload `"ff03::10,1"` to subscribe to the multicast address. Idempotent.

- **URL:** `/api/thread/groups/assign`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "device_addr": "fd12:3456:789a::1",
  "group_addr": "ff03::10"
}
```

| Field       | Type   | Required | Description              |
|-------------|--------|----------|--------------------------|
| device_addr | string | Yes      | Device IPv6 address      |
| group_addr  | string | Yes      | Group multicast address  |

- **Response:** `{"ok":true}` on success, `404` if group not found, `507` if member limit (8) reached

---

#### Remove Device from Group

Removes a device from a group. The device receives a CoAP PUT `/group` with payload `"ff03::10,0"` to unsubscribe.

- **URL:** `/api/thread/groups/assign`
- **Method:** `DELETE`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "device_addr": "fd12:3456:789a::1",
  "group_addr": "ff03::10"
}
```

| Field       | Type   | Required | Description              |
|-------------|--------|----------|--------------------------|
| device_addr | string | Yes      | Device IPv6 address      |
| group_addr  | string | Yes      | Group multicast address  |

- **Response:** `{"ok":true}` on success, `404` if group or member not found

---

#### Send Group Command

Sends a CoAP PUT to a multicast address. All subscribed devices respond. Fire-and-forget.

- **URL:** `/api/thread/groups/command`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "addr": "ff03::10",
  "resource": "outdoor",
  "on": false
}
```

| Field    | Type    | Required | Description                          |
|----------|---------|----------|--------------------------------------|
| addr     | string  | Yes      | Group multicast address              |
| resource | string  | Yes      | `"beacon"` or `"outdoor"`            |
| on       | boolean | Yes      | `true` = on, `false` = off           |

- **Response:** `{"ok":true}` on success

---

### Scenes

#### Get All Scenes

Returns all configured scenes.

- **URL:** `/api/scenes`
- **Method:** `GET`
- **Response:**

```json
[
  {
    "id": "scene-1",
    "name": "Evening Mood",
    "icon": "🌅",
    "actions": {
      "light": "on",
      "mode": "simulation",
      "schema": "schema_02.csv",
      "devices": [
        {
          "id": "matter-001",
          "state": "on"
        }
      ]
    }
  },
  {
    "id": "scene-2",
    "name": "Night Mode",
    "icon": "🌙",
    "actions": {
      "light": "on",
      "mode": "night"
    }
  }
]
```

**Scene Object:**

| Field   | Type   | Description                        |
|---------|--------|------------------------------------|
| id      | string | Unique scene identifier            |
| name    | string | Scene name                         |
| icon    | string | Emoji icon for the scene           |
| actions | object | Actions to execute (see below)     |

**Actions Object:**

| Field   | Type   | Optional | Description                              |
|---------|--------|----------|------------------------------------------|
| light   | string | Yes      | `"on"` or `"off"`                        |
| mode    | string | Yes      | `"day"`, `"night"`, or `"simulation"`    |
| schema  | string | Yes      | Schema filename (e.g., `schema_01.csv`)  |
| devices | array  | Yes      | Array of device actions (see below)      |

**Device Action Object:**

| Field | Type   | Description                      |
|-------|--------|----------------------------------|
| id    | string | Device ID                        |
| state | string | `"on"` or `"off"`                |

---

#### Create/Update Scene

Creates a new scene or updates an existing one.

- **URL:** `/api/scenes`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "id": "scene-1",
  "name": "Evening Mood",
  "icon": "🌅",
  "actions": {
    "light": "on",
    "mode": "simulation",
    "schema": "schema_02.csv",
    "devices": [
      {
        "id": "matter-001",
        "state": "on"
      }
    ]
  }
}
```

- **Response:** `200 OK` on success

---

#### Delete Scene

Deletes a scene.

- **URL:** `/api/scenes`
- **Method:** `DELETE`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "id": "scene-1"
}
```

| Field | Type   | Required | Description   |
|-------|--------|----------|---------------|
| id    | string | Yes      | Scene ID      |

- **Response:** `200 OK` on success

---

#### Activate Scene

Executes all actions of a scene.

- **URL:** `/api/scenes/activate`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "id": "scene-1"
}
```

| Field | Type   | Required | Description   |
|-------|--------|----------|---------------|
| id    | string | Yes      | Scene ID      |

- **Response:** `200 OK` on success

---

### Input

#### Simulate Button Input

Remotely triggers a button action as if pressed on the physical device. Useful for controlling the menu UI via the web interface.

- **URL:** `/api/input`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "action": "button_up",
  "value": ""
}
```

| Field  | Type   | Required | Description                                                      |
|--------|--------|----------|------------------------------------------------------------------|
| action | string | Yes      | Action name to execute (see list below)                          |
| value  | string | No       | Optional value passed to the action handler (default: empty)     |

**Available actions:**

| Action         | Description                              |
|----------------|------------------------------------------|
| `button_up`    | Navigate up in the menu                  |
| `button_down`  | Navigate down in the menu                |
| `button_left`  | Adjust value left (e.g. cycle selection) |
| `button_right` | Adjust value right                       |
| `button_select`| Activate the selected menu item          |
| `button_back`  | Go back to the previous screen           |

- **Response:** `200 OK` on success, `400 Bad Request` if `action` field is missing

**Notes:**
- Actions are dispatched via the heimdall action manager
- Any registered action can be triggered, not just button actions
- The value field is passed through to the action handler but is currently unused for button actions

---

## WebSocket

### Connection

- **URL:** `ws://{host}/ws` or `wss://{host}/ws`
- **Protocol:** JSON messages

The WebSocket connection is used for real-time status updates. The client should reconnect automatically if the connection is lost (recommended: 3 second delay).

---

### Client to Server Messages

#### Request Status

Requests the current system status.

```json
{
  "type": "getStatus"
}
```

---

### Server to Client Messages

#### Status Update

Sent in response to `getStatus` or when status changes.

```json
{
  "type": "status",
  "on": true,
  "mode": "simulation",
  "schema": "schema_01.csv",
  "color": {
    "r": 255,
    "g": 240,
    "b": 220
  }
}
```

| Field  | Type    | Description                          |
|--------|---------|--------------------------------------|
| type   | string  | Always `"status"`                    |
| on     | boolean | Light power state                    |
| mode   | string  | Current mode (day/night/simulation)  |
| schema | string  | Active schema filename               |
| color  | object  | Current RGB color (optional)         |

---

#### Color Update

Sent when the current color changes (during simulation).

```json
{
  "type": "color",
  "r": 255,
  "g": 200,
  "b": 150
}
```

| Field | Type   | Description              |
|-------|--------|--------------------------|
| type  | string | Always `"color"`         |
| r     | number | Red value (0-255)        |
| g     | number | Green value (0-255)      |
| b     | number | Blue value (0-255)       |

---

#### Thread: Resource State Change (RFC 7641 Observe)

Sent whenever a device's `/beacon` or `/outdoor` state changes. Fired by the C6 after receiving a CoAP Observe notification (CON 2.05 Content) from the H2.

```json
{
  "type": "thread_state",
  "addr": "fd12:3456:789a::1",
  "resource": "beacon",
  "on": true
}
```

| Field    | Type    | Description                           |
|----------|---------|---------------------------------------|
| type     | string  | Always `"thread_state"`               |
| addr     | string  | Device IPv6 address                   |
| resource | string  | `"beacon"` or `"outdoor"`             |
| on       | boolean | New resource state                    |

**Notes:**
- Also fires once when the Observe subscription is first established (initial state).
- The first notification after `GET /api/thread/devices` or a new device announcement reflects the actual current hardware state.
- When a group command (multicast PUT) is issued, each device sends an individual `thread_state` event after switching — the number of events received is a confirmation count for the broadcast.

---

#### Thread: Device Announced

Sent when a Thread device sends a CoAP `/announce` (on network join) or when one is added manually.

```json
{
  "type": "thread_device",
  "name": "Leuchtturm West",
  "addr": "fd12:3456:789a::1"
}
```

---

#### Thread: Device Removed

Sent when a device is deleted via the REST API.

```json
{
  "type": "thread_device_removed",
  "addr": "fd12:3456:789a::1"
}
```

---

#### Thread: Capabilities Discovered

Sent after a successful CoAP `/.well-known/core` query (triggered automatically on announce, manual add, or when a device comes back online after being unreachable). Also serves as the **reachable-again** signal after a `thread_unreachable` event.

```json
{
  "type": "thread_capabilities",
  "addr": "fd12:3456:789a::1",
  "beacon": true,
  "outdoor": true
}
```

---

#### Thread: Device Unreachable

Sent when CoAP Observe times out for a device — i.e. the device stopped sending notifications and OpenThread gave up retransmitting. Emitted at most once per outage regardless of how many resources (beacon, outdoor) the device had registered observers for.

```json
{
  "type": "thread_unreachable",
  "addr": "fd12:3456:789a::1"
}
```

| Field | Type   | Description                        |
| ----- | ------ | ---------------------------------- |
| type  | string | Always `"thread_unreachable"`      |
| addr  | string | IPv6 address of the offline device |

The next `thread_capabilities` event for the same `addr` signals that the device is reachable again. At that point CoAP Observe is automatically re-registered.

---

#### Thread: Group Added

Sent when a group is created.

```json
{
  "type": "thread_group_added",
  "name": "Alle Aussenlampen",
  "addr": "ff03::10"
}
```

---

#### Thread: Group Removed

Sent when a group is deleted.

```json
{
  "type": "thread_group_removed",
  "addr": "ff03::10"
}
```

---

#### Thread: Device Assigned to Group

Sent when a device is added to a group.

```json
{
  "type": "thread_group_assigned",
  "device": "fd12:3456:789a::1",
  "group": "ff03::10"
}
```

---

#### Thread: Device Removed from Group

Sent when a device is removed from a group.

```json
{
  "type": "thread_group_unassigned",
  "device": "fd12:3456:789a::1",
  "group": "ff03::10"
}
```

---

#### WiFi Status Update

Sent when WiFi connection status changes.

```json
{
  "type": "wifi",
  "connected": true,
  "ip": "192.168.1.100",
  "rssi": -45
}
```

| Field     | Type    | Description                           |
|-----------|---------|---------------------------------------|
| type      | string  | Always `"wifi"`                       |
| connected | boolean | Whether connected to a network        |
| ip        | string  | Assigned IP address (if connected)    |
| rssi      | number  | Signal strength in dBm (if connected) |

---

## Static Files

The web interface files should be served from the `/spiffs/www/` directory:

| Path                 | Description                                    |
|----------------------|------------------------------------------------|
| `/`                  | Serves `index.html` (or `captive.html` in AP mode) |
| `/index.html`        | Main HTML file (full interface)                |
| `/captive.html`      | Captive portal (WiFi setup only)               |
| `/css/shared.css`    | Shared styles for all pages                    |
| `/css/index.css`     | Styles for main interface                      |
| `/css/captive.css`   | Styles for captive portal                      |
| `/js/wifi-shared.js` | Shared WiFi configuration logic                |
| `/js/*.js`           | JavaScript modules                             |

---

## Captive Portal

When the ESP32 is in Access Point (AP) mode (no WiFi configured or connection failed), it should serve the captive portal:

### Behavior

1. **AP Mode Activation:**
   - ESP32 creates an access point (e.g., "marklin-setup")
   - DNS server redirects all requests to the ESP32's IP (captive portal detection)

2. **Captive Portal Detection:**
   - Respond to common captive portal detection URLs:
     - `/generate_204` (Android)
     - `/hotspot-detect.html` (Apple)
     - `/connecttest.txt` (Windows)
   - Return redirect or serve `captive.html`

3. **Serving Files in AP Mode:**
   - `/` → `captive.html`
   - `/captive.html` → Captive portal page
   - `/js/wifi-shared.js` → WiFi functions
   - API endpoints remain the same (`/api/wifi/*`)

4. **After Successful Configuration:**
   - ESP32 attempts to connect to the configured network
   - If successful, switch to Station mode and serve `index.html`
   - If failed, remain in AP mode

### Recommended AP Settings

| Setting       | Value                    |
|---------------|--------------------------|
| SSID          | `marklin-setup`          |
| Password      | None (open) or `marklin` |
| IP Address    | `192.168.4.1`            |
| Gateway       | `192.168.4.1`            |
| Subnet        | `255.255.255.0`          |

---

## Error Handling

All endpoints should return appropriate HTTP status codes:

| Code | Description                           |
|------|---------------------------------------|
| 200  | Success                               |
| 400  | Bad Request (invalid input)           |
| 404  | Not Found (resource doesn't exist)    |
| 500  | Internal Server Error                 |

Error responses should include a JSON body with an error message:

```json
{
  "error": "Description of what went wrong"
}
```
