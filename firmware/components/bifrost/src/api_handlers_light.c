#include "bifrost/api_handlers.h"
#include "bifrost/api_handlers_util.h"
#include "bifrost/common.h"
#include "led_segment.h"
#include "message_manager.h"
#include "persistence_manager.h"
#include "storage.h"

#include <cJSON.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <string.h>

static const char *TAG = "api_light";

// ============================================================================
// Light Control API
// ============================================================================

esp_err_t api_light_power_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/light/power");
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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
                msg.data.settings.value.int_value = -1; // Unknown mode
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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

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

    extern led_segment_t segments[LED_SEGMENT_MAX_LEN];
    extern size_t segment_count;
    size_t required_size = sizeof(segments) * segment_count;

    cJSON *json = cJSON_CreateObject();

    persistence_manager_t pm;
    if (persistence_manager_init(&pm, "led_config") == ESP_OK)
    {
        persistence_manager_get_blob(&pm, "segments", segments, required_size, NULL);
        uint8_t segment_count = persistence_manager_get_int(&pm, "segment_count", 0);
        persistence_manager_deinit(&pm);

        cJSON *segments_arr = cJSON_CreateArray();
        for (uint8_t i = 0; i < segment_count; ++i)
        {
            cJSON *seg = cJSON_CreateObject();
            cJSON_AddStringToObject(seg, "name", segments[i].name);
            cJSON_AddNumberToObject(seg, "start", segments[i].start);
            cJSON_AddNumberToObject(seg, "leds", segments[i].leds);
            cJSON_AddItemToArray(segments_arr, seg);
        }
        cJSON_AddItemToObject(json, "segments", segments_arr);
    }
    else
    {
        cJSON_AddItemToObject(json, "segments", cJSON_CreateArray());
    }

    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    esp_err_t res = send_json_response(req, response);
    free(response);
    return res;
}

esp_err_t api_wled_config_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/wled/config");
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

    char *buf = heap_caps_malloc(MAX_BODY_SIZE, MALLOC_CAP_DEFAULT);
    if (!buf)
        return send_error_response(req, 500, "Memory allocation failed");
    int total = 0, ret;
    while (total < MAX_BODY_SIZE - 1)
    {
        ret = httpd_req_recv(req, buf + total, MAX_BODY_SIZE - 1 - total);
        if (ret <= 0)
            break;
        total += ret;
    }
    buf[total] = '\0';

    ESP_LOGI(TAG, "Received WLED config: %s", buf);

    cJSON *json = cJSON_Parse(buf);
    free(buf);

    if (!json)
    {
        return send_error_response(req, 400, "Invalid JSON");
    }

    cJSON *segments_arr = cJSON_GetObjectItem(json, "segments");
    if (!cJSON_IsArray(segments_arr))
    {
        cJSON_Delete(json);
        return send_error_response(req, 400, "Missing segments array");
    }

    extern led_segment_t segments[LED_SEGMENT_MAX_LEN];
    extern size_t segment_count;
    size_t count = cJSON_GetArraySize(segments_arr);
    if (count > LED_SEGMENT_MAX_LEN)
        count = LED_SEGMENT_MAX_LEN;
    segment_count = count;
    for (size_t i = 0; i < LED_SEGMENT_MAX_LEN; ++i)
    {
        cJSON *seg = cJSON_GetArrayItem(segments_arr, i);
        cJSON *name = cJSON_GetObjectItem(seg, "name");
        cJSON *start = cJSON_GetObjectItem(seg, "start");
        cJSON *leds = cJSON_GetObjectItem(seg, "leds");
        if (cJSON_IsString(name) && cJSON_IsNumber(start) && cJSON_IsNumber(leds) && i < count)
        {
            strncpy(segments[i].name, name->valuestring, sizeof(segments[i].name) - 1);
            segments[i].name[sizeof(segments[i].name) - 1] = '\0';
            segments[i].start = (uint16_t)start->valuedouble;
            segments[i].leds = (uint16_t)leds->valuedouble;
        }
        else
        {
            // Invalid entry, skip or set defaults
            segments[i].name[0] = '\0';
            segments[i].start = 0;
            segments[i].leds = 0;
        }
    }
    cJSON_Delete(json);

    persistence_manager_t pm;
    if (persistence_manager_init(&pm, "led_config") == ESP_OK)
    {
        persistence_manager_set_blob(&pm, "segments", segments, sizeof(led_segment_t) * segment_count);
        persistence_manager_set_int(&pm, "segment_count", (int32_t)segment_count);
        persistence_manager_deinit(&pm);
    }

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

    // Read schema file
    char path[128];
    snprintf(path, sizeof(path), "%s", filename);

    int line_count = 0;
    char **lines = read_lines_filtered(path, &line_count);

    set_cors_headers(req);
    httpd_resp_set_type(req, "text/csv");

    if (!lines || line_count == 0)
    {
        return httpd_resp_sendstr(req, "");
    }

    // Calculate total length
    size_t total_len = 0;
    for (int i = 0; i < line_count; ++i)
        total_len += strlen(lines[i]) + 1;
    char *csv = heap_caps_malloc(total_len + 1, MALLOC_CAP_DEFAULT);
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
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

    // Extract filename from URI
    if (!req)
    {
        ESP_LOGE(TAG, "Request pointer is NULL");
        return send_error_response(req, 500, "Internal error: req is NULL");
    }
    const char *uri = req->uri;
    ESP_LOGI(TAG, "Request URI: %s", uri ? uri : "(null)");
    if (!uri)
    {
        ESP_LOGE(TAG, "Request URI is NULL");
        return send_error_response(req, 400, "Invalid schema path (no URI)");
    }
    const char *filename = strrchr(uri, '/');
    if (filename == NULL || filename[1] == '\0')
    {
        ESP_LOGE(TAG, "Could not extract filename from URI: %s", uri);
        return send_error_response(req, 400, "Invalid schema path");
    }
    filename++;
    ESP_LOGI(TAG, "Extracted filename: %s", filename);

    // Dynamically read POST body
    char *buf = heap_caps_malloc(MAX_BODY_SIZE, MALLOC_CAP_DEFAULT);
    if (!buf)
    {
        ESP_LOGE(TAG, "Memory allocation failed for POST body");
        return send_error_response(req, 500, "Memory allocation failed");
    }
    int total = 0, ret;
    while (total < MAX_BODY_SIZE - 1)
    {
        ret = httpd_req_recv(req, buf + total, MAX_BODY_SIZE - 1 - total);
        if (ret <= 0)
            break;
        total += ret;
    }
    buf[total] = '\0';

    ESP_LOGI(TAG, "Saving schema %s, size: %d bytes", filename, total);

    // Split CSV body into line array
    int line_count = 0;
    // Count lines
    for (int i = 0; i < total; ++i)
        if (buf[i] == '\n')
            line_count++;
    if (total > 0 && buf[total - 1] != '\n')
        line_count++; // last line without \n

    char **lines = (char **)heap_caps_malloc(line_count * sizeof(char *), MALLOC_CAP_DEFAULT);
    int idx = 0;
    char *saveptr = NULL;
    char *line = strtok_r(buf, "\n", &saveptr);
    while (line && idx < line_count)
    {
        // Ignore empty lines
        if (line[0] != '\0')
            lines[idx++] = heap_caps_strdup(line, MALLOC_CAP_DEFAULT);
        line = strtok_r(NULL, "\n", &saveptr);
    }
    int actual_count = idx;
    esp_err_t err = write_lines(filename, lines, actual_count);
    for (int i = 0; i < actual_count; ++i)
        free(lines[i]);
    free(lines);
    set_cors_headers(req);

    if (err != ESP_OK)
        return send_error_response(req, 500, "Failed to save schema");
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}
