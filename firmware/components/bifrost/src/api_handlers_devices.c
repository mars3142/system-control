#include "bifrost/api_handlers.h"
#include "bifrost/api_handlers_util.h"

#include <esp_heap_caps.h>
#include <esp_log.h>
#include <string.h>

static const char *TAG = "api_devices";

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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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
