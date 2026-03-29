# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32 firmware for a model railway system control unit, targeting ESP32-C6 microcontrollers. Built with ESP-IDF 5.5. Includes a Svelte 5 web UI (`website/`) and a REST/WebSocket/MQTT API.

## ESP-IDF Environment Setup

ESP-IDF is **not** on PATH by default. The installation lives at a non-standard path; `export.sh` must be sourced before `idf.py` is available.

```bash
# One-time per shell session — activates idf.py, xtensa/riscv toolchains, etc.
. /Users/mars3142/.espressif/v5.5.3/esp-idf/export.sh
```

If `export.sh` cannot find the Python environment it will print an error like
`doesn't exist! Please run the install script`. In that case invoke `idf.py`
directly via the venv Python, which bypasses the PATH check:

```bash
IDF_PYTHON_ENV_PATH=/Users/mars3142/.espressif/tools/python/v5.5.3/venv \
IDF_TOOLS_PATH=/Users/mars3142/.espressif/tools \
/Users/mars3142/.espressif/tools/python/v5.5.3/venv/bin/python \
  /Users/mars3142/.espressif/v5.5.3/esp-idf/tools/idf.py <command>
```

Key paths:
- IDF root: `/Users/mars3142/.espressif/v5.5.3/esp-idf/`
- Python venv: `/Users/mars3142/.espressif/tools/python/v5.5.3/venv/`
- IDF tools: `/Users/mars3142/.espressif/tools/`

## Build Commands

```bash
# After sourcing export.sh:

# Firmware (ESP32-C6, default target)
idf.py -DIDF_TARGET=esp32c6 build

# Flash everything (overwrites SPIFFS — paired Thread devices list is lost)
idf.py -p <PORT> flash

# Flash only the app binary (preserves SPIFFS and NVS — use this during development)
idf.py -p <PORT> app-flash

# Flash only the SPIFFS partition (updates menu.json / web assets without touching NVS)
idf.py -p <PORT> storage-flash

idf.py -p <PORT> flash monitor

# Release build (ESP32-C6 only)
make release
# Equivalent: idf.py -B build-release -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.release" -DIDF_TARGET=esp32c6 fullclean build size

# Web UI
cd website && npm install && npm run build
cd website && npm run test
```

## Architecture

- `main/` — Firmware entry (`app_main()` in `main.cpp`, core app task in `app_task.cpp`)
- `components/` — ESP-IDF components with isolated business logic:
  - **bifrost** (`api-server`) — HTTP REST + WebSocket server, mDNS, static file serving
  - **connectivity-manager** — WiFi, BLE, captive portal / DNS hijacking
  - **led-manager** — WS2812 strip control, effects, status LED
  - **message-manager** — Observer/broadcast bus for cross-component events
  - **persistence-manager** — NVS abstraction with namespace-scoped typed read/write
  - **mercedes** — Dynamic menu data model (C++ singleton, JSON-driven)
  - **hermes** — u8g2 display rendering (menu, splash, screensaver)
  - **heimdall** — Button/action manager with callback registration
  - **simulator** — Day/night light cycle simulation from CSV schedules
  - **iris** — Thread network device manager; split into `iris.c` (public API + state), `iris_storage.c`, `iris_coap.c`, `iris_discovery.c`, `iris_master.c`, `iris_inventory.c`; shared internal header at `include/iris/iris_internal.h`
- `storage/` — Runtime SPIFFS content: `menu.json`, `schema_*.csv`, `www/` (web assets)
- `website/` — Svelte 5 + Vite + Tailwind web UI

### Multi-target Support

Two targets are supported with distinct pin assignments and `sdkconfig` defaults:
- `sdkconfig.defaults` — base (shared)
- `sdkconfig.defaults.esp32s3` — S3 overrides (obsolete)
- `sdkconfig.defaults.esp32c6` — C6 overrides (includes WiFi enable/antenna GPIO)

Do not assume one target's pins or settings apply to the other.

### Task Structure

FreeRTOS tasks in priority order:
1. `app_main()` — hardware init, starts `app_task`
2. `app_task` — main logic on core 1, priority `tskIDLE_PRIORITY + 5`, 8KB stack
3. `display_update_task` — async I2C display writes

## Conventions

### C/C++ Style
- Braces on new line in C files; follow existing formatting per file.
- Each source file has a local `static const char *TAG = "..."` for `ESP_LOGI/W/E`.
- HTTP handlers must use `set_cors_headers`, `send_json_response`, and `send_error_response` for consistency.
- Cross-component state changes go through `message_manager_post(...)`, not direct coupling.
- Persist settings via `persistence_manager` with a dedicated namespace per feature.

### URI Handler Registration Order (api-server)
1. Specific `/api/...` handlers
2. Wildcard API handlers
3. Captive-portal detection handlers
4. `OPTIONS /api/*`
5. Static fallback `/*` (last)

WebSocket handling must be initialized before API handler registration.

### Static Asset Serving
Preserve correct HTTP headers: content-type, CORS, and `Content-Encoding: gzip` when serving `.gz` variants.

## Key Reference Files

- `README-API.md` — full REST API documentation
- `README-menu.md` — `menu.json` schema
- `README-captive.md` — captive portal behavior
- `components/bifrost/src/api_server.c` — server setup and handler registration
- `components/bifrost/src/api_handlers.c` — REST endpoint implementations
- `components/message-manager/src/message_manager.c`
- `components/persistence-manager/src/persistence_manager.c`
- `partitions.csv` — flash layout (NVS 16KB, APP 2MB, SPIFFS 1.8MB)
