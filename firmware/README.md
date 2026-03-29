## System Control

ESP32 firmware for a model railway system control unit.

### Hardware

Custom PCB with **ESP32-C6** (primary target). The ESP32-S3 target exists in the codebase but is no longer actively maintained.

### Features

- WS2812 LED strip control with day/night simulation (CSV schedules)
- SSD1306 OLED display with dynamic menu (driven by `menu.json`)
- REST + WebSocket + MQTT API
- WiFi station mode + captive portal AP mode for initial setup
- Thread network integration (**Iris** component) — manages ESP32-H2 model railway accessories via CoAP
- mDNS hostname: `system-control.local`

### Repository Layout

| Path | Description |
|------|-------------|
| `main/` | Firmware entry point (`app_main`, `app_task`) |
| `components/` | ESP-IDF components (see below) |
| `storage/` | SPIFFS runtime content: `menu.json`, CSV schemas, web assets |
| `website/` | Svelte 5 + Vite + Tailwind web UI |
| `partitions.csv` | Flash layout (NVS 16 KB, APP 2 MB, SPIFFS 1.8 MB) |

### Components

| Component | Description |
|-----------|-------------|
| `bifrost` | HTTP REST + WebSocket server, mDNS, static file serving |
| `connectivity-manager` | WiFi, BLE, captive portal / DNS hijacking |
| `led-manager` | WS2812 strip control, effects, status LED |
| `message-manager` | Observer/broadcast bus for cross-component events |
| `persistence-manager` | NVS abstraction with namespace-scoped typed read/write |
| `mercedes` | Dynamic menu data model (C++ singleton, JSON-driven) |
| `hermes` | u8g2 display rendering (menu, splash, screensaver) |
| `heimdall` | Button/action manager with callback registration |
| `simulator` | Day/night light cycle simulation from CSV schedules |
| `iris` | Thread network device manager (ESP32-H2 accessories via CoAP) |

### Build

Requires ESP-IDF 5.5.3. See `CLAUDE.md` for full build instructions.

```bash
# Source ESP-IDF environment
. /Users/mars3142/.espressif/v5.5.3/esp-idf/export.sh

# Build
idf.py -DIDF_TARGET=esp32c6 build

# Flash app only (preserves SPIFFS — Thread device list intact)
idf.py -p <PORT> app-flash
```

### Documentation

| File | Description |
|------|-------------|
| `README-API.md` | REST + WebSocket API reference |
| `README-thread.md` | Thread network (Iris) architecture and H2 implementation guide |
| `README-menu.md` | `menu.json` schema |
| `README-captive.md` | Captive portal behaviour |
| `CLAUDE.md` | Claude Code build and architecture reference |
