#pragma once

#include <cJSON.h>
#include <esp_http_server.h>
#include <stdint.h>

#define MAX_BODY_SIZE 4096

esp_err_t set_cors_headers(httpd_req_t *req);
esp_err_t send_json_response(httpd_req_t *req, const char *json);
esp_err_t send_error_response(httpd_req_t *req, int status_code, const char *message);
esp_err_t options_handler(httpd_req_t *req);

bool is_valid(const cJSON *string);
char *heap_caps_strdup(const char *src, uint32_t caps);
const char *get_mime_type(const char *path);
