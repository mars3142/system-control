# Project Guidelines

## Code Style
- Keep existing C/C++ formatting as in this repo (brace-on-new-line in C files, compact designated initializers where already used).
- Use `ESP_LOGI/W/E` with a local `TAG` constant in each source file.
- For HTTP API handlers, prefer existing helper functions for response consistency: `set_cors_headers`, `send_json_response`, and `send_error_response`.
- Keep changes minimal and component-local. Avoid cross-component refactors unless explicitly requested.

## Architecture
- Firmware entry and orchestration live in `main/`.
- Business logic is split into ESP-IDF components under `components/` (notably `api-server`, `message-manager`, `persistence-manager`, `connectivity-manager`, `led-manager`).
- Web UI sources are in `website/`; static assets are served by `api-server` from `CONFIG_API_SERVER_STATIC_FILES_PATH`.
- Storage-related runtime files and schema CSVs are in `storage/`.

## Build and Test
- Default firmware build: `idf.py build`
- Flash: `idf.py -p <PORT> flash`
- Flash + monitor: `idf.py -p <PORT> flash monitor`
- Release build (from `Makefile`):
  `idf.py -B build-release -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.release" -DIDF_TARGET=esp32c6 fullclean build size`
- Web UI build:
  `cd website && npm install && npm run build`
- Web UI tests:
  `cd website && npm run test`

## Conventions
- In `api-server`, URI handler registration order is critical:
  1) specific `/api/...` handlers
  2) wildcard API handlers
  3) captive-portal detection handlers
  4) `OPTIONS /api/*`
  5) static fallback `/*` last
- In `api_server.c`, initialize WebSocket handling before API handler registration.
- Use `message_manager_post(...)` for cross-component state updates instead of direct coupling.
- Persist settings via `persistence_manager` using explicit namespaces per feature area.

## Pitfalls
- This workspace supports multiple ESP targets (`esp32s3`, `esp32c6`) with different defaults (`sdkconfig.defaults.*`). Do not assume one target's pins/settings for all builds.
- Keep `build/` artifacts out of manual edits; source of truth is under `main/`, `components/`, `storage/`, and `website/`.
- For static assets, preserve correct HTTP headers (content type, CORS, and `Content-Encoding: gzip` when serving `.gz` variants).

## Key References
- `README.md`
- `README-API.md`
- `README-captive.md`
- `components/api-server/src/api_handlers.c`
- `components/api-server/src/api_server.c`
- `components/message-manager/src/message_manager.c`
- `components/persistence-manager/src/persistence_manager.c`
- `website/package.json`
- `Makefile`
