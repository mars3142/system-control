#include "bifrost/api_handlers_util.h"

#include <esp_heap_caps.h>
#include <string.h>

esp_err_t set_cors_headers(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    return ESP_OK;
}

esp_err_t send_json_response(httpd_req_t *req, const char *json)
{
    set_cors_headers(req);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, json);
}

esp_err_t send_error_response(httpd_req_t *req, int status_code, const char *message)
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

esp_err_t options_handler(httpd_req_t *req)
{
    set_cors_headers(req);
    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

bool is_valid(const cJSON *string)
{
    return string && cJSON_IsString(string) && string->valuestring && strlen(string->valuestring) > 0;
}

char *heap_caps_strdup(const char *src, uint32_t caps)
{
    if (!src)
        return NULL;
    size_t len = strlen(src) + 1;
    char *dst = heap_caps_malloc(len, caps);
    if (dst)
        memcpy(dst, src, len);
    return dst;
}

const char *get_mime_type(const char *path)
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
