---
description: "Use when implementing or changing ESP-IDF HTTP handlers, REST routes, captive portal flows, static file serving, CORS handling, or WebSocket-adjacent API behavior in bifrost."
name: "Bifrost (API Server) Instructions"
applyTo: "components/bifrost/**/*.{c,h}"
---
# Bifrost (API Server) Guidelines

- Keep handler registration order stable in api_handlers.c:
  1) specific /api routes
  2) wildcard API routes
  3) captive-portal detection routes
  4) OPTIONS /api/*
  5) static fallback /* last
- Preserve response consistency by reusing helper functions:
  - set_cors_headers
  - send_json_response
  - send_error_response
- In api_server.c, initialize WebSocket handling before registering API handlers.
- Keep static asset behavior HTTP-correct:
  - content type by original file type
  - CORS headers for browser usage
  - Content-Encoding: gzip when serving .gz assets
- Prefer localized changes in bifrost only; avoid cross-component coupling for state changes.
- For state updates, use message_manager_post instead of direct control flow into other components.
- Add clear ESP_LOGI/W/E messages with the local TAG for new request paths and error branches.
