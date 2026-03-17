---
description: "Generate a complete new API endpoint (handler, registration, response structure) that follows project conventions, with minimal setup required."
name: "Add API Endpoint"
argument-hint: "Endpoint path and purpose (e.g., GET /api/device/status, POST /api/scenes/create)"
agent: "agent"
---
# Add API Endpoint

Create a new API endpoint that follows this project's conventions:

**Input Requirements:**
- Endpoint path and HTTP method (GET, POST, DELETE, etc.)
- Brief description of endpoint purpose and data flow
- Response shape (JSON keys/types expected)
- Any cross-component dependencies (message_manager, persistence_manager, etc.)

**Deliverables:**
1. Handler function stub with:
   - Proper ESP_LOGI entry logging
   - Input validation (where applicable)
   - Consistent error responses via send_error_response
   - CORS headers via set_cors_headers
   - JSON output via send_json_response
2. Handler registration in api_handlers_register(), placed in correct order relative to existing routes
3. Example request/response JSON
4. Any required persistence_manager namespaces or message_manager posting

**Reference Style From:**
- [components/api-server/src/api_handlers.c](components/api-server/src/api_handlers.c#L65)—api_capabilities_get_handler, api_wifi_status_handler
- [.github/instructions/api-server.instructions.md](.github/instructions/api-server.instructions.md)—handler order, helper function patterns

**Output Format:**
```c
// Handler function
esp_err_t api_YOURNAME_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "...");
  // implementation
}

// Registration snippet (paste into api_handlers_register)
httpd_uri_t YOURNAME = {...};
err = httpd_register_uri_handler(server, &YOURNAME);
```
