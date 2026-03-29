#include "bifrost/api_handlers.h"
#include "bifrost/api_handlers_util.h"
#include "heimdall/action_manager.h"

#include <cJSON.h>
#include <esp_http_server.h>
#include <esp_log.h>

static const char *TAG = "api_handlers";

// ============================================================================
// Input API (Heimdall)
// ============================================================================

static esp_err_t api_input_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/input");

    char body[MAX_BODY_SIZE];
    int received = httpd_req_recv(req, body, sizeof(body) - 1);
    if (received <= 0)
        return send_error_response(req, 400, "Empty body");
    body[received] = '\0';

    cJSON *json = cJSON_Parse(body);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    const cJSON *action = cJSON_GetObjectItem(json, "action");
    if (!cJSON_IsString(action) || !action->valuestring[0])
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing or empty 'action' field");
    }

    const cJSON *value = cJSON_GetObjectItem(json, "value");
    const char *value_str = (cJSON_IsString(value)) ? value->valuestring : NULL;

    action_manager_execute(action->valuestring, value_str);

    cJSON_Delete(json);
    return send_json_response(req, "{\"ok\":true}");
}

// ============================================================================
// Handler Registration
// ============================================================================

esp_err_t api_handlers_register(httpd_handle_t server)
{
    esp_err_t err;

    // Capabilities
    httpd_uri_t capabilities_get = {
        .uri = "/api/capabilities", .method = HTTP_GET, .handler = api_capabilities_get_handler};
    err = httpd_register_uri_handler(server, &capabilities_get);
    if (err != ESP_OK)
        return err;

    // WiFi endpoints
    httpd_uri_t wifi_scan = {.uri = "/api/wifi/scan", .method = HTTP_GET, .handler = api_wifi_scan_handler};
    err = httpd_register_uri_handler(server, &wifi_scan);
    if (err != ESP_OK)
        return err;

    httpd_uri_t wifi_config = {.uri = "/api/wifi/config", .method = HTTP_POST, .handler = api_wifi_config_handler};
    err = httpd_register_uri_handler(server, &wifi_config);
    if (err != ESP_OK)
        return err;

    httpd_uri_t wifi_status = {.uri = "/api/wifi/status", .method = HTTP_GET, .handler = api_wifi_status_handler};
    err = httpd_register_uri_handler(server, &wifi_status);
    if (err != ESP_OK)
        return err;

    // Light endpoints
    httpd_uri_t light_power = {.uri = "/api/light/power", .method = HTTP_POST, .handler = api_light_power_handler};
    err = httpd_register_uri_handler(server, &light_power);
    if (err != ESP_OK)
        return err;

    httpd_uri_t light_thunder = {
        .uri = "/api/light/thunder", .method = HTTP_POST, .handler = api_light_thunder_handler};
    err = httpd_register_uri_handler(server, &light_thunder);
    if (err != ESP_OK)
        return err;

    httpd_uri_t light_mode = {.uri = "/api/light/mode", .method = HTTP_POST, .handler = api_light_mode_handler};
    err = httpd_register_uri_handler(server, &light_mode);
    if (err != ESP_OK)
        return err;

    httpd_uri_t light_schema = {.uri = "/api/light/schema", .method = HTTP_POST, .handler = api_light_schema_handler};
    err = httpd_register_uri_handler(server, &light_schema);
    if (err != ESP_OK)
        return err;

    httpd_uri_t light_status = {.uri = "/api/light/status", .method = HTTP_GET, .handler = api_light_status_handler};
    err = httpd_register_uri_handler(server, &light_status);
    if (err != ESP_OK)
        return err;

    // WLED config endpoints
    httpd_uri_t wled_config_get = {
        .uri = "/api/wled/config", .method = HTTP_GET, .handler = api_wled_config_get_handler};
    err = httpd_register_uri_handler(server, &wled_config_get);
    if (err != ESP_OK)
        return err;

    httpd_uri_t wled_config_post = {
        .uri = "/api/wled/config", .method = HTTP_POST, .handler = api_wled_config_post_handler};
    err = httpd_register_uri_handler(server, &wled_config_post);
    if (err != ESP_OK)
        return err;

    // Schema endpoints (wildcard)
    httpd_uri_t schema_get = {.uri = "/api/schema/*", .method = HTTP_GET, .handler = api_schema_get_handler};
    err = httpd_register_uri_handler(server, &schema_get);
    if (err != ESP_OK)
        return err;

    httpd_uri_t schema_post = {.uri = "/api/schema/*", .method = HTTP_POST, .handler = api_schema_post_handler};
    err = httpd_register_uri_handler(server, &schema_post);
    if (err != ESP_OK)
        return err;

    // Devices endpoints
    httpd_uri_t devices_scan = {.uri = "/api/devices/scan", .method = HTTP_GET, .handler = api_devices_scan_handler};
    err = httpd_register_uri_handler(server, &devices_scan);
    if (err != ESP_OK)
        return err;

    httpd_uri_t devices_pair = {.uri = "/api/devices/pair", .method = HTTP_POST, .handler = api_devices_pair_handler};
    err = httpd_register_uri_handler(server, &devices_pair);
    if (err != ESP_OK)
        return err;

    httpd_uri_t devices_paired = {
        .uri = "/api/devices/paired", .method = HTTP_GET, .handler = api_devices_paired_handler};
    err = httpd_register_uri_handler(server, &devices_paired);
    if (err != ESP_OK)
        return err;

    httpd_uri_t devices_update = {
        .uri = "/api/devices/update", .method = HTTP_POST, .handler = api_devices_update_handler};
    err = httpd_register_uri_handler(server, &devices_update);
    if (err != ESP_OK)
        return err;

    httpd_uri_t devices_unpair = {
        .uri = "/api/devices/unpair", .method = HTTP_POST, .handler = api_devices_unpair_handler};
    err = httpd_register_uri_handler(server, &devices_unpair);
    if (err != ESP_OK)
        return err;

    httpd_uri_t devices_toggle = {
        .uri = "/api/devices/toggle", .method = HTTP_POST, .handler = api_devices_toggle_handler};
    err = httpd_register_uri_handler(server, &devices_toggle);
    if (err != ESP_OK)
        return err;

    httpd_uri_t devices_toggle_all = {
        .uri = "/api/devices/toggle_all", .method = HTTP_POST, .handler = api_devices_toggle_all_handler};
    err = httpd_register_uri_handler(server, &devices_toggle_all);
    if (err != ESP_OK)
        return err;

    // Scenes endpoints
    httpd_uri_t scenes_get = {.uri = "/api/scenes", .method = HTTP_GET, .handler = api_scenes_get_handler};
    err = httpd_register_uri_handler(server, &scenes_get);
    if (err != ESP_OK)
        return err;

    httpd_uri_t scenes_post = {.uri = "/api/scenes", .method = HTTP_POST, .handler = api_scenes_post_handler};
    err = httpd_register_uri_handler(server, &scenes_post);
    if (err != ESP_OK)
        return err;

    httpd_uri_t scenes_delete = {.uri = "/api/scenes", .method = HTTP_DELETE, .handler = api_scenes_delete_handler};
    err = httpd_register_uri_handler(server, &scenes_delete);
    if (err != ESP_OK)
        return err;

    httpd_uri_t scenes_activate = {
        .uri = "/api/scenes/activate", .method = HTTP_POST, .handler = api_scenes_activate_handler};
    err = httpd_register_uri_handler(server, &scenes_activate);
    if (err != ESP_OK)
        return err;

    // Input endpoint (Heimdall action dispatch)
    httpd_uri_t input_post = {.uri = "/api/input", .method = HTTP_POST, .handler = api_input_handler};
    err = httpd_register_uri_handler(server, &input_post);
    if (err != ESP_OK)
        return err;

    // Captive portal detection endpoints
    httpd_uri_t captive_generate_204 = {
        .uri = "/generate_204", .method = HTTP_GET, .handler = api_captive_portal_handler};
    err = httpd_register_uri_handler(server, &captive_generate_204);
    if (err != ESP_OK)
        return err;

    httpd_uri_t captive_hotspot = {
        .uri = "/hotspot-detect.html", .method = HTTP_GET, .handler = api_captive_portal_handler};
    err = httpd_register_uri_handler(server, &captive_hotspot);
    if (err != ESP_OK)
        return err;

    httpd_uri_t captive_connecttest = {
        .uri = "/connecttest.txt", .method = HTTP_GET, .handler = api_captive_portal_handler};
    err = httpd_register_uri_handler(server, &captive_connecttest);
    if (err != ESP_OK)
        return err;

    // OPTIONS handler for CORS preflight (wildcard)
    httpd_uri_t options = {.uri = "/api/*", .method = HTTP_OPTIONS, .handler = options_handler};
    err = httpd_register_uri_handler(server, &options);
    if (err != ESP_OK)
        return err;

    // Static file handler (must be last due to wildcard)
    httpd_uri_t static_files = {.uri = "/*", .method = HTTP_GET, .handler = api_static_file_handler};
    err = httpd_register_uri_handler(server, &static_files);
    if (err != ESP_OK)
        return err;

    ESP_LOGI(TAG, "All API handlers registered");
    return ESP_OK;
}
