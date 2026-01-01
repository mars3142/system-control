# System Control - API Documentation

This document describes all REST API endpoints and WebSocket messages required for the ESP32 firmware implementation.

## Table of Contents

- [REST API Endpoints](#rest-api-endpoints)
  - [WiFi](#wifi)
  - [Light Control](#light-control)
  - [WLED Configuration](#wled-configuration)
  - [Schema](#schema)
  - [Devices](#devices)
  - [Scenes](#scenes)
- [WebSocket](#websocket)
  - [Connection](#connection)
  - [Client to Server Messages](#client-to-server-messages)
  - [Server to Client Messages](#server-to-client-messages)

---

## REST API Endpoints

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
| on     | boolean | Current power state                  |
| mode   | string  | Current mode (day/night/simulation)  |
| schema | string  | Active schema filename               |
| color  | object  | Current RGB color being displayed    |

---

### WLED Configuration

#### Get WLED Configuration

Returns the current WLED configuration including host and all segments.

- **URL:** `/api/wled/config`
- **Method:** `GET`
- **Response:**

```json
{
  "host": "192.168.1.100",
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
| host               | string | WLED host address (IP or hostname)       |
| segments           | array  | List of LED segments                     |
| segments[].name    | string | Optional segment name                    |
| segments[].start   | number | Start LED index (0-based)                |
| segments[].leds    | number | Number of LEDs in this segment           |

---

#### Save WLED Configuration

Saves the WLED configuration.

- **URL:** `/api/wled/config`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "host": "192.168.1.100",
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
| host               | string | Yes      | WLED host address (IP or hostname)       |
| segments           | array  | Yes      | List of LED segments (can be empty)      |
| segments[].name    | string | No       | Optional segment name                    |
| segments[].start   | number | Yes      | Start LED index (0-based)                |
| segments[].leds    | number | Yes      | Number of LEDs in this segment           |

- **Response:** `200 OK` on success, `400 Bad Request` on validation error

**Notes:**
- The firmware uses this configuration to communicate with a WLED controller
- Segments are mapped to the WLED JSON API segment control
- Changes are persisted to NVS (non-volatile storage)
- The host can be an IP address (e.g., `192.168.1.100`) or hostname (e.g., `wled.local`)

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

### Devices

#### Scan for Devices

Scans for available Matter devices to pair.

- **URL:** `/api/devices/scan`
- **Method:** `GET`
- **Response:**

```json
[
  {
    "id": "matter-001",
    "type": "light",
    "name": "Matter Lamp"
  },
  {
    "id": "matter-002",
    "type": "sensor",
    "name": "Temperature Sensor"
  }
]
```

| Field | Type   | Description                                   |
|-------|--------|-----------------------------------------------|
| id    | string | Unique device identifier                      |
| type  | string | Device type: `light`, `sensor`, `unknown`     |
| name  | string | Device name (can be empty)                    |

---

#### Pair Device

Pairs a discovered device.

- **URL:** `/api/devices/pair`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "id": "matter-001",
  "name": "Living Room Lamp"
}
```

| Field | Type   | Required | Description                  |
|-------|--------|----------|------------------------------|
| id    | string | Yes      | Device ID from scan          |
| name  | string | Yes      | User-defined device name     |

- **Response:** `200 OK` on success

---

#### Get Paired Devices

Returns list of all paired devices.

- **URL:** `/api/devices/paired`
- **Method:** `GET`
- **Response:**

```json
[
  {
    "id": "matter-001",
    "type": "light",
    "name": "Living Room Lamp"
  }
]
```

| Field | Type   | Description                               |
|-------|--------|-------------------------------------------|
| id    | string | Unique device identifier                  |
| type  | string | Device type: `light`, `sensor`, `unknown` |
| name  | string | User-defined device name                  |

---

#### Update Device Name

Updates the name of a paired device.

- **URL:** `/api/devices/update`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "id": "matter-001",
  "name": "New Device Name"
}
```

| Field | Type   | Required | Description            |
|-------|--------|----------|------------------------|
| id    | string | Yes      | Device ID              |
| name  | string | Yes      | New device name        |

- **Response:** `200 OK` on success

---

#### Unpair Device

Removes a paired device.

- **URL:** `/api/devices/unpair`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "id": "matter-001"
}
```

| Field | Type   | Required | Description   |
|-------|--------|----------|---------------|
| id    | string | Yes      | Device ID     |

- **Response:** `200 OK` on success

---

#### Toggle Device

Toggles a device (e.g., light on/off).

- **URL:** `/api/devices/toggle`
- **Method:** `POST`
- **Content-Type:** `application/json`
- **Request Body:**

```json
{
  "id": "matter-001"
}
```

| Field | Type   | Required | Description   |
|-------|--------|----------|---------------|
| id    | string | Yes      | Device ID     |

- **Response:** `200 OK` on success

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

The web interface files should be served from the `/storage/www/` directory:

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
