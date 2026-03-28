#include "bifrost/api_handlers.h"
#include "bifrost/api_handlers_util.h"

#include <esp_log.h>
#include <esp_wifi.h>
#include <string.h>
#include <sys/stat.h>

static const char *TAG = "api_static";

// ============================================================================
// Static file serving
// ============================================================================

esp_err_t api_static_file_handler(httpd_req_t *req)
{
    char filepath[CONFIG_HTTPD_MAX_URI_LEN + 16];
    char gz_filepath[CONFIG_HTTPD_MAX_URI_LEN + 20];

    const char *uri = req->uri;
    wifi_mode_t mode = 0;
    esp_wifi_get_mode(&mode);
    // Always serve captive.html in AP mode
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

    bool use_gzip = false;
    const char *served_path = filepath;

    written = snprintf(gz_filepath, sizeof(gz_filepath), "%s.gz", filepath);
    if (written >= 0 && (size_t)written < sizeof(gz_filepath))
    {
        struct stat gz_st;
        if (stat(gz_filepath, &gz_st) == 0)
        {
            use_gzip = true;
            served_path = gz_filepath;
        }
    }

    ESP_LOGI(TAG, "Serving static file: %s%s", served_path, use_gzip ? " (gzip)" : "");

    // Check if file exists
    struct stat st;
    if (stat(served_path, &st) != 0)
    {
        ESP_LOGW(TAG, "File not found: %s", served_path);
        return send_error_response(req, 404, "File not found");
    }

    // Open and serve file
    FILE *f = fopen(served_path, "rb");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", served_path);
        return send_error_response(req, 500, "Failed to open file");
    }

    set_cors_headers(req);
    httpd_resp_set_type(req, get_mime_type(filepath));
    if (use_gzip)
    {
        httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    }

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

    // Serve captive.html directly (status 200, text/html)
    const char *base_path = CONFIG_API_SERVER_STATIC_FILES_PATH;
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/captive.html", base_path);
    FILE *f = fopen(filepath, "r");
    if (!f)
    {
        ESP_LOGE(TAG, "captive.html not found: %s", filepath);
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "Captive portal not available");
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
