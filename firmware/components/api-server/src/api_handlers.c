#include "api_handlers.h"
#include "common.h"
#include "message_manager.h"

#include <cJSON.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <persistence_manager.h>
#include <string.h>
#include <sys/stat.h>

static const char *TAG = "api_handlers";

// Helper function to set CORS headers
static esp_err_t set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    return ESP_OK;
}

// Helper function to send JSON response
static esp_err_t send_json_response(httpd_req_t *req, const char *json)
{
    set_cors_headers(req);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, json);
}

// Helper function to send error response
static esp_err_t send_error_response(httpd_req_t *req, int status_code, const char *message)
{
    set_cors_headers(req);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_status(req, status_code == 400   ? "400 Bad Request"
                               : status_code == 404 ? "404 Not Found"
                                                    : "500 Internal Server Error");
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "{\"error\":\"%s\"}", message);
    return httpd_resp_sendstr(req, buffer);
}

// OPTIONS handler for CORS preflight
static esp_err_t options_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// ============================================================================
// Capabilities API
// ============================================================================

esp_err_t api_capabilities_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/capabilities");

    // Thread nur für esp32c6 oder esp32h2 verfügbar
    bool thread = false;
#if defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
    thread = true;
#endif
    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "thread", thread);
    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    esp_err_t res = send_json_response(req, response);
    free(response);
    return res;
}

// ============================================================================
// WiFi API
// ============================================================================

esp_err_t api_wifi_scan_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/wifi/scan");

    wifi_scan_config_t scan_config = {.ssid = NULL, .bssid = NULL, .channel = 0, .show_hidden = true};
    esp_err_t err = esp_wifi_scan_start(&scan_config, true);
    if (err != ESP_OK)
    {
        return send_error_response(req, 500, "WiFi scan failed");
    }

    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);
    wifi_ap_record_t *ap_list = calloc(ap_num, sizeof(wifi_ap_record_t));
    if (!ap_list)
    {
        return send_error_response(req, 500, "Memory allocation failed");
    }
    esp_wifi_scan_get_ap_records(&ap_num, ap_list);

    cJSON *json = cJSON_CreateArray();
    for (int i = 0; i < ap_num; i++)
    {
        if (ap_list[i].ssid[0] != '\0')
        {
            cJSON *entry = cJSON_CreateObject();
            cJSON_AddStringToObject(entry, "ssid", (const char *)ap_list[i].ssid);
            cJSON_AddNumberToObject(entry, "rssi", ap_list[i].rssi);
            bool secure = ap_list[i].authmode != WIFI_AUTH_OPEN;
            cJSON_AddBoolToObject(entry, "secure", secure);
            cJSON_AddItemToArray(json, entry);
        }
    }
    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    free(ap_list);
    esp_err_t res = send_json_response(req, response);
    free(response);
    return res;
}

static void reboot_task(void *param)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}

static bool is_valid(const cJSON *string)
{
    return string && cJSON_IsString(string) && string->valuestring && strlen(string->valuestring) > 0;
}

esp_err_t api_wifi_config_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/wifi/config");

    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    cJSON *json = cJSON_Parse(buf);
    if (json)
    {
        cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
        cJSON *pw = cJSON_GetObjectItem(json, "password");
        if (is_valid(ssid) && is_valid(pw))
        {
            persistence_manager_t pm;
            if (persistence_manager_init(&pm, "wifi_config") == ESP_OK)
            {
                persistence_manager_set_string(&pm, "ssid", ssid->valuestring);
                persistence_manager_set_string(&pm, "password", pw->valuestring);
                persistence_manager_deinit(&pm);
            }
        }
        if (is_valid(pw))
        {
            size_t pwlen = strlen(pw->valuestring);
            char *masked = malloc(pwlen + 1);
            if (masked)
            {
                memset(masked, '*', pwlen);
                masked[pwlen] = '\0';
                cJSON_ReplaceItemInObject(json, "password", cJSON_CreateString(masked));
                char *logstr = cJSON_PrintUnformatted(json);
                ESP_LOGI(TAG, "Received WiFi config: %s", logstr);
                free(logstr);
                free(masked);
            }
            else
            {
                ESP_LOGI(TAG, "Received WiFi config: %s", buf);
            }
        }
        else
        {
            ESP_LOGI(TAG, "Received WiFi config: %s", buf);
        }
        cJSON_Delete(json);
    }
    else
    {
        ESP_LOGI(TAG, "Received WiFi config: %s", buf);
    }

    // Define a reboot task function
    xTaskCreate(reboot_task, "reboot_task", 2048, NULL, 5, NULL);

    set_cors_headers(req);
    httpd_resp_set_status(req, "200 OK");
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_wifi_status_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/wifi/status");

    wifi_ap_record_t ap_info;
    bool connected = false;
    char ssid[33] = "";
    char ip[16] = "";
    int rssi = 0;

    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
    {
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
        {
            connected = true;
            strncpy(ssid, (const char *)ap_info.ssid, sizeof(ssid) - 1);
            rssi = ap_info.rssi;
        }
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif)
        {
            esp_netif_ip_info_t ip_info;
            if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK)
            {
                snprintf(ip, sizeof(ip), "%d.%d.%d.%d", esp_ip4_addr1(&ip_info.ip), esp_ip4_addr2(&ip_info.ip),
                         esp_ip4_addr3(&ip_info.ip), esp_ip4_addr4(&ip_info.ip));
            }
        }
    }

    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "connected", connected);
    cJSON_AddStringToObject(json, "ssid", ssid);
    cJSON_AddStringToObject(json, "ip", ip);
    cJSON_AddNumberToObject(json, "rssi", rssi);
    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    esp_err_t res = send_json_response(req, response);
    free(response);
    return res;
}

// ============================================================================
// Light Control API
// ============================================================================

esp_err_t api_light_power_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/light/power");

    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Received light power: %s", buf);

    cJSON *json = cJSON_Parse(buf);
    if (json)
    {
        cJSON *active = cJSON_GetObjectItem(json, "on");
        if (cJSON_IsBool(active))
        {
            message_t msg = {};
            msg.type = MESSAGE_TYPE_SETTINGS;
            msg.data.settings.type = SETTINGS_TYPE_BOOL;
            strncpy(msg.data.settings.key, "light_active", sizeof(msg.data.settings.key) - 1);
            msg.data.settings.value.bool_value = cJSON_IsTrue(active);
            message_manager_post(&msg, pdMS_TO_TICKS(100));
        }
        cJSON_Delete(json);
    }

    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_light_thunder_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/light/thunder");

    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Received thunder setting: %s", buf);

    // TODO: Parse JSON and control thunder effect
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_light_mode_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/light/mode");

    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Received light mode: %s", buf);

    cJSON *json = cJSON_Parse(buf);
    if (json)
    {
        cJSON *mode = cJSON_GetObjectItem(json, "mode");
        if (cJSON_IsString(mode))
        {
            message_t msg = {};
            msg.type = MESSAGE_TYPE_SETTINGS;
            msg.data.settings.type = SETTINGS_TYPE_INT;
            strncpy(msg.data.settings.key, "light_mode", sizeof(msg.data.settings.key) - 1);
            if (strcmp(mode->valuestring, "simulation") == 0)
            {
                msg.data.settings.value.int_value = 0;
            }
            else if (strcmp(mode->valuestring, "day") == 0)
            {
                msg.data.settings.value.int_value = 1;
            }
            else if (strcmp(mode->valuestring, "night") == 0)
            {
                msg.data.settings.value.int_value = 2;
            }
            else
            {
                msg.data.settings.value.int_value = -1; // Unbekannter Modus
            }
            message_manager_post(&msg, pdMS_TO_TICKS(100));
        }
        cJSON_Delete(json);
    }

    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_light_schema_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/light/schema");

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Received schema setting: %s", buf);

    cJSON *json = cJSON_Parse(buf);
    if (json)
    {
        cJSON *schema_file = cJSON_GetObjectItem(json, "schema");
        if (cJSON_IsString(schema_file))
        {
            int schema_id = 0;
            sscanf(schema_file->valuestring, "schema_%d.csv", &schema_id);

            message_t msg = {};
            msg.type = MESSAGE_TYPE_SETTINGS;
            msg.data.settings.type = SETTINGS_TYPE_INT;
            strncpy(msg.data.settings.key, "light_variant", sizeof(msg.data.settings.key) - 1);
            msg.data.settings.value.int_value = schema_id;
            message_manager_post(&msg, pdMS_TO_TICKS(100));
        }
        cJSON_Delete(json);
    }

    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_light_status_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/light/status");
    cJSON *json = create_light_status_json();
    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    esp_err_t res = send_json_response(req, response);
    free(response);
    return res;
}

// ============================================================================
// LED Configuration API
// ============================================================================

esp_err_t api_wled_config_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/wled/config");

    // TODO: Implement actual LED config retrieval
    const char *response = "{"
                           "\"segments\":["
                           "{\"name\":\"Main Light\",\"start\":0,\"leds\":60},"
                           "{\"name\":\"Accent Light\",\"start\":60,\"leds\":30}"
                           "]"
                           "}";
    return send_json_response(req, response);
}

esp_err_t api_wled_config_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/wled/config");

    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Received WLED config: %s", buf);

    // TODO: Parse JSON and save LED configuration
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

// ============================================================================
// Schema API
// ============================================================================

esp_err_t api_schema_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/schema/*");

    // Extract filename from URI
    const char *uri = req->uri;
    const char *filename = strrchr(uri, '/');
    if (filename == NULL)
    {
        return send_error_response(req, 400, "Invalid schema path");
    }
    filename++; // Skip the '/'

    ESP_LOGI(TAG, "Requested schema: %s", filename);

    // Schema-Datei lesen
    char path[128];
    snprintf(path, sizeof(path), "%s", filename);

    int line_count = 0;
    extern char **read_lines_filtered(const char *filename, int *out_count);
    extern void free_lines(char **lines, int count);
    char **lines = read_lines_filtered(path, &line_count);

    set_cors_headers(req);
    httpd_resp_set_type(req, "text/csv");

    if (!lines || line_count == 0)
    {
        return httpd_resp_sendstr(req, "");
    }

    // Gesamtlänge berechnen
    size_t total_len = 0;
    for (int i = 0; i < line_count; ++i)
        total_len += strlen(lines[i]) + 1;
    char *csv = malloc(total_len + 1);
    char *p = csv;
    for (int i = 0; i < line_count; ++i)
    {
        size_t l = strlen(lines[i]);
        memcpy(p, lines[i], l);
        p += l;
        *p++ = '\n';
    }
    *p = '\0';
    free_lines(lines, line_count);
    esp_err_t res = httpd_resp_sendstr(req, csv);
    free(csv);
    return res;
}

esp_err_t api_schema_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/schema/*");

    // Extract filename from URI
    const char *uri = req->uri;
    const char *filename = strrchr(uri, '/');
    if (filename == NULL)
    {
        return send_error_response(req, 400, "Invalid schema path");
    }
    filename++;

    char buf[2048];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Saving schema %s, size: %d bytes", filename, ret);

    // TODO: Save schema to storage
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

// ============================================================================
// Devices API (Matter)
// ============================================================================

esp_err_t api_devices_scan_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/devices/scan");

    // TODO: Implement Matter device scanning
    const char *response = "["
                           "{\"id\":\"matter-001\",\"type\":\"light\",\"name\":\"Matter Lamp\"},"
                           "{\"id\":\"matter-002\",\"type\":\"sensor\",\"name\":\"Temperature Sensor\"}"
                           "]";
    return send_json_response(req, response);
}

esp_err_t api_devices_pair_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/pair");

    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Pairing device: %s", buf);

    // TODO: Implement Matter device pairing
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_devices_paired_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/devices/paired");

    // TODO: Get list of paired devices
    const char *response = "["
                           "{\"id\":\"matter-001\",\"type\":\"light\",\"name\":\"Living Room Lamp\"}"
                           "]";
    return send_json_response(req, response);
}

esp_err_t api_devices_update_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/update");

    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Updating device: %s", buf);

    // TODO: Update device name
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_devices_unpair_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/unpair");

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Unpairing device: %s", buf);

    // TODO: Unpair device
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_devices_toggle_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/toggle");

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Toggling device: %s", buf);

    // TODO: Toggle device
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

// ============================================================================
// Scenes API
// ============================================================================

esp_err_t api_scenes_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/scenes");

    // TODO: Get scenes from storage
    const char *response = "["
                           "{"
                           "\"id\":\"scene-1\","
                           "\"name\":\"Evening Mood\","
                           "\"icon\":\"🌅\","
                           "\"actions\":{"
                           "\"light\":\"on\","
                           "\"mode\":\"simulation\","
                           "\"schema\":\"schema_02.csv\""
                           "}"
                           "},"
                           "{"
                           "\"id\":\"scene-2\","
                           "\"name\":\"Night Mode\","
                           "\"icon\":\"🌙\","
                           "\"actions\":{"
                           "\"light\":\"on\","
                           "\"mode\":\"night\""
                           "}"
                           "}"
                           "]";
    return send_json_response(req, response);
}

esp_err_t api_scenes_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/scenes");

    char buf[512];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Creating/updating scene: %s", buf);

    // TODO: Save scene to storage
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_scenes_delete_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "DELETE /api/scenes");

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Deleting scene: %s", buf);

    // TODO: Delete scene from storage
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_scenes_activate_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/scenes/activate");

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    ESP_LOGI(TAG, "Activating scene: %s", buf);

    // TODO: Activate scene
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

// ============================================================================
// Static file serving
// ============================================================================

// Get MIME type from file extension
static const char *get_mime_type(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (ext == NULL)
        return "text/plain";

    if (strcmp(ext, ".html") == 0)
        return "text/html";
    if (strcmp(ext, ".css") == 0)
        return "text/css";
    if (strcmp(ext, ".js") == 0)
        return "application/javascript";
    if (strcmp(ext, ".json") == 0)
        return "application/json";
    if (strcmp(ext, ".png") == 0)
        return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".svg") == 0)
        return "image/svg+xml";
    if (strcmp(ext, ".ico") == 0)
        return "image/x-icon";
    if (strcmp(ext, ".csv") == 0)
        return "text/csv";

    return "text/plain";
}

esp_err_t api_static_file_handler(httpd_req_t *req)
{
    char filepath[CONFIG_HTTPD_MAX_URI_LEN + 16];

    const char *uri = req->uri;
    wifi_mode_t mode = 0;
    esp_wifi_get_mode(&mode);
    // Im AP-Modus immer captive.html ausliefern
    if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
    {
        if (strcmp(uri, "/") == 0 || strcmp(uri, "/index.html") == 0)
        {
            uri = "/captive.html";
        }
    }
    else
    {
        // Default to index.html for root
        if (strcmp(uri, "/") == 0)
        {
            uri = "/index.html";
        }
    }

    const char *base_path = CONFIG_API_SERVER_STATIC_FILES_PATH;
    int written = snprintf(filepath, sizeof(filepath), "%s%s", base_path, uri);
    if (written < 0 || (size_t)written >= sizeof(filepath))
    {
        ESP_LOGE(TAG, "URI too long: %s", uri);
        return send_error_response(req, 400, "URI too long");
    }

    ESP_LOGI(TAG, "Serving static file: %s", filepath);

    // Check if file exists
    struct stat st;
    if (stat(filepath, &st) != 0)
    {
        ESP_LOGW(TAG, "File not found: %s", filepath);
        return send_error_response(req, 404, "File not found");
    }

    // Open and serve file
    FILE *f = fopen(filepath, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        return send_error_response(req, 500, "Failed to open file");
    }

    set_cors_headers(req);
    httpd_resp_set_type(req, get_mime_type(filepath));

    char buf[512];
    size_t read_bytes;
    while ((read_bytes = fread(buf, 1, sizeof(buf), f)) > 0)
    {
        if (httpd_resp_send_chunk(req, buf, read_bytes) != ESP_OK)
        {
            fclose(f);
            ESP_LOGE(TAG, "Failed to send file chunk");
            return ESP_FAIL;
        }
    }

    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // End response
    return ESP_OK;
}

// ============================================================================
// Captive portal detection
// ============================================================================

esp_err_t api_captive_portal_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Captive portal detection: %s", req->uri);

    // captive.html direkt ausliefern (Status 200, text/html)
    const char *base_path = CONFIG_API_SERVER_STATIC_FILES_PATH;
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/captive.html", base_path);
    FILE *f = fopen(filepath, "r");
    if (!f)
    {
        ESP_LOGE(TAG, "captive.html not found: %s", filepath);
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "Captive Portal nicht verfügbar");
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "text/html");
    char buf[512];
    size_t read_bytes;
    while ((read_bytes = fread(buf, 1, sizeof(buf), f)) > 0)
    {
        if (httpd_resp_send_chunk(req, buf, read_bytes) != ESP_OK)
        {
            fclose(f);
            ESP_LOGE(TAG, "Failed to send captive chunk");
            return ESP_FAIL;
        }
    }
    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
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
