#include "bifrost/api_handlers.h"
#include "bifrost/api_handlers_util.h"

#include <cJSON.h>
#include <esp_log.h>
#include <string.h>

#if defined(CONFIG_IRIS_ENABLED)
#include "iris/iris.h"
#endif

static const char *TAG = "api_devices";

// ============================================================================
// Devices API (Iris / Thread)
// ============================================================================

esp_err_t api_devices_scan_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/devices/scan");

#if defined(CONFIG_IRIS_ENABLED)
    iris_device_t devices[8];
    int count = iris_scan(devices, 8);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; i++)
    {
        char eui_str[17];
        iris_eui64_to_str(devices[i].p.eui64, eui_str, sizeof(eui_str));

        cJSON *obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "id",   eui_str);
        cJSON_AddStringToObject(obj, "name", devices[i].p.name);
        cJSON_AddNumberToObject(obj, "capabilities", devices[i].p.capabilities);
        cJSON_AddItemToArray(arr, obj);
    }
    char *resp = cJSON_PrintUnformatted(arr);
    cJSON_Delete(arr);
    esp_err_t res = send_json_response(req, resp);
    free(resp);
    return res;
#else
    return send_json_response(req, "[]");
#endif
}

esp_err_t api_devices_pair_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/pair");

    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
        return send_error_response(req, 400, "Failed to receive request body");
    buf[ret] = '\0';

#if defined(CONFIG_IRIS_ENABLED)
    cJSON *json = cJSON_Parse(buf);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    cJSON *id_item   = cJSON_GetObjectItem(json, "id");
    cJSON *name_item = cJSON_GetObjectItem(json, "name");

    if (!cJSON_IsString(id_item))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing field 'id'");
    }

    uint8_t eui64[IRIS_EUI64_LEN];
    if (!iris_str_to_eui64(id_item->valuestring, eui64))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Invalid device id (expected 16-char hex EUI-64)");
    }

    const char *name = cJSON_IsString(name_item) ? name_item->valuestring : id_item->valuestring;
    esp_err_t err = iris_pair(eui64, name);
    cJSON_Delete(json);

    set_cors_headers(req);
    if (err == ESP_OK)
        return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    if (err == ESP_ERR_NOT_SUPPORTED)
        return send_error_response(req, 403, "Not allowed on Backup unit");
    return send_error_response(req, 500, "Pairing failed");
#else
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"not_supported\"}");
#endif
}

esp_err_t api_devices_paired_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/devices/paired");

#if defined(CONFIG_IRIS_ENABLED)
    iris_device_t devices[CONFIG_IRIS_MAX_DEVICES];
    int count = iris_get_paired(devices, CONFIG_IRIS_MAX_DEVICES);

    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < count; i++)
    {
        char eui_str[17];
        iris_eui64_to_str(devices[i].p.eui64, eui_str, sizeof(eui_str));

        cJSON *obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "id",     eui_str);
        cJSON_AddStringToObject(obj, "name",   devices[i].p.name);
        cJSON_AddNumberToObject(obj, "capabilities", devices[i].p.capabilities);
        cJSON_AddNumberToObject(obj, "state",  devices[i].p.state);
        cJSON_AddBoolToObject(obj,   "online", devices[i].online);
        cJSON_AddItemToArray(arr, obj);
    }
    char *resp = cJSON_PrintUnformatted(arr);
    cJSON_Delete(arr);
    esp_err_t res = send_json_response(req, resp);
    free(resp);
    return res;
#else
    return send_json_response(req, "[]");
#endif
}

esp_err_t api_devices_update_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/update");

    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
        return send_error_response(req, 400, "Failed to receive request body");
    buf[ret] = '\0';

#if defined(CONFIG_IRIS_ENABLED)
    cJSON *json = cJSON_Parse(buf);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    cJSON *id_item   = cJSON_GetObjectItem(json, "id");
    cJSON *name_item = cJSON_GetObjectItem(json, "name");

    if (!cJSON_IsString(id_item) || !cJSON_IsString(name_item))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing fields 'id' or 'name'");
    }

    uint8_t eui64[IRIS_EUI64_LEN];
    esp_err_t err = ESP_ERR_NOT_FOUND;
    if (iris_str_to_eui64(id_item->valuestring, eui64))
        err = iris_rename(eui64, name_item->valuestring);
    cJSON_Delete(json);

    set_cors_headers(req);
    if (err == ESP_OK)
        return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    if (err == ESP_ERR_NOT_FOUND)
        return send_error_response(req, 404, "Device not found");
    return send_error_response(req, 500, "Update failed");
#else
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"not_supported\"}");
#endif
}

esp_err_t api_devices_unpair_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/unpair");

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
        return send_error_response(req, 400, "Failed to receive request body");
    buf[ret] = '\0';

#if defined(CONFIG_IRIS_ENABLED)
    cJSON *json = cJSON_Parse(buf);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    cJSON *id_item = cJSON_GetObjectItem(json, "id");
    if (!cJSON_IsString(id_item))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing field 'id'");
    }

    uint8_t eui64[IRIS_EUI64_LEN];
    esp_err_t err = ESP_ERR_NOT_FOUND;
    if (iris_str_to_eui64(id_item->valuestring, eui64))
        err = iris_unpair(eui64);
    cJSON_Delete(json);

    set_cors_headers(req);
    if (err == ESP_OK)
        return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    if (err == ESP_ERR_NOT_FOUND)
        return send_error_response(req, 404, "Device not found");
    return send_error_response(req, 500, "Unpair failed");
#else
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"not_supported\"}");
#endif
}

esp_err_t api_devices_toggle_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/toggle");

    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
        return send_error_response(req, 400, "Failed to receive request body");
    buf[ret] = '\0';

#if defined(CONFIG_IRIS_ENABLED)
    cJSON *json = cJSON_Parse(buf);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    cJSON *id_item  = cJSON_GetObjectItem(json, "id");
    cJSON *cap_item = cJSON_GetObjectItem(json, "cap");

    if (!cJSON_IsString(id_item) || !cJSON_IsNumber(cap_item))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing fields 'id' or 'cap'");
    }

    uint8_t eui64[IRIS_EUI64_LEN];
    uint8_t cap = (uint8_t)cap_item->valuedouble;
    esp_err_t err = ESP_ERR_NOT_FOUND;
    if (iris_str_to_eui64(id_item->valuestring, eui64))
        err = iris_toggle(eui64, cap);
    cJSON_Delete(json);

    set_cors_headers(req);
    if (err == ESP_OK)
        return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    if (err == ESP_ERR_NOT_FOUND)
        return send_error_response(req, 404, "Device not found");
    return send_error_response(req, 500, "Toggle failed");
#else
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"not_supported\"}");
#endif
}

esp_err_t api_devices_toggle_all_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/devices/toggle_all");

    char buf[64];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
        return send_error_response(req, 400, "Failed to receive request body");
    buf[ret] = '\0';

#if defined(CONFIG_IRIS_ENABLED)
    cJSON *json = cJSON_Parse(buf);
    if (!json)
        return send_error_response(req, 400, "Invalid JSON");

    cJSON *cap_item   = cJSON_GetObjectItem(json, "cap");
    cJSON *state_item = cJSON_GetObjectItem(json, "state");
    if (!cJSON_IsNumber(cap_item) || !cJSON_IsNumber(state_item))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing fields 'cap' and/or 'state'");
    }

    uint8_t cap  = (uint8_t)cap_item->valuedouble;
    bool    on   = (state_item->valuedouble != 0.0);
    cJSON_Delete(json);

    esp_err_t err = iris_set_all(cap, on);
    set_cors_headers(req);
    if (err == ESP_OK)
        return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
    return send_error_response(req, 500, "Multicast set failed");
#else
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"not_supported\"}");
#endif
}

// ============================================================================
// Scenes API (placeholder — not yet implemented)
// ============================================================================

esp_err_t api_scenes_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/scenes");
    return send_json_response(req, "[]");
}

esp_err_t api_scenes_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/scenes");
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"not_implemented\"}");
}

esp_err_t api_scenes_delete_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "DELETE /api/scenes");
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"not_implemented\"}");
}

esp_err_t api_scenes_activate_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/scenes/activate");
    set_cors_headers(req);
    return httpd_resp_sendstr(req, "{\"status\":\"not_implemented\"}");
}
