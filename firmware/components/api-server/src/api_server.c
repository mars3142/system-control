#include "api_server.h"
#include "api_handlers.h"
#include "websocket_handler.h"

#include "common.h"
#include "storage.h"
#include <esp_http_server.h>
#include <esp_log.h>
#include <mdns.h>
#include <string.h>

static const char *TAG = "api_server";

static httpd_handle_t s_server = NULL;
static api_server_config_t s_config = API_SERVER_CONFIG_DEFAULT();

static esp_err_t init_mdns(const char *hostname)
{
    esp_err_t err = mdns_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize mDNS: %s", esp_err_to_name(err));
        return err;
    }

    err = mdns_hostname_set(hostname);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set mDNS hostname: %s", esp_err_to_name(err));
        return err;
    }

    err = mdns_instance_name_set("System Control");
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set mDNS instance name: %s", esp_err_to_name(err));
        return err;
    }

    // Add HTTP service
    err = mdns_service_add("System Control Web Server", "_http", "_tcp", s_config.port, NULL, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add mDNS HTTP service: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "mDNS initialized: %s.local", hostname);
    return ESP_OK;
}

static esp_err_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = s_config.port;
    config.lru_purge_enable = true;
    config.max_uri_handlers = 32;
    config.max_open_sockets = 7;
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP server on port %d", config.server_port);

    esp_err_t err = httpd_start(&s_server, &config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(err));
        return err;
    }

    // WebSocket-Handler explizit vor allen API-Handlern
    err = websocket_handler_init(s_server);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize WebSocket handler: %s", esp_err_to_name(err));
        httpd_stop(s_server);
        s_server = NULL;
        return err;
    }

    // Register API handlers
    err = api_handlers_register(s_server);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register API handlers: %s", esp_err_to_name(err));
        httpd_stop(s_server);
        s_server = NULL;
        return err;
    }

    // Common initialization
    common_init();

    ESP_LOGI(TAG, "HTTP server started successfully");
    return ESP_OK;
}

esp_err_t api_server_start(const api_server_config_t *config)
{
    if (s_server != NULL)
    {
        ESP_LOGW(TAG, "Server already running");
        return ESP_ERR_INVALID_STATE;
    }

    if (config != NULL)
    {
        s_config = *config;
    }

    initialize_storage();

    // Initialize mDNS
    esp_err_t err = init_mdns(s_config.hostname);
    if (err != ESP_OK)
    {
        return err;
    }

    // Start web server
    err = start_webserver();
    if (err != ESP_OK)
    {
        mdns_free();
        return err;
    }

    return ESP_OK;
}

esp_err_t api_server_stop(void)
{
    if (s_server == NULL)
    {
        ESP_LOGW(TAG, "Server not running");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = httpd_stop(s_server);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to stop HTTP server: %s", esp_err_to_name(err));
        return err;
    }

    s_server = NULL;
    mdns_free();

    ESP_LOGI(TAG, "Server stopped");
    return ESP_OK;
}

bool api_server_is_running(void)
{
    return s_server != NULL;
}

httpd_handle_t api_server_get_handle(void)
{
    return s_server;
}

esp_err_t api_server_ws_broadcast(const char *message)
{
    if (s_server == NULL)
    {
        return ESP_ERR_INVALID_STATE;
    }
    return websocket_broadcast(s_server, message);
}

esp_err_t api_server_ws_broadcast_status(bool on, const char *mode, const char *schema, uint8_t r, uint8_t g, uint8_t b)
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer),
             "{\"type\":\"status\",\"on\":%s,\"mode\":\"%s\",\"schema\":\"%s\","
             "\"color\":{\"r\":%d,\"g\":%d,\"b\":%d}}",
             on ? "true" : "false", mode, schema, r, g, b);
    return api_server_ws_broadcast(buffer);
}

esp_err_t api_server_ws_broadcast_color(uint8_t r, uint8_t g, uint8_t b)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "{\"type\":\"color\",\"r\":%d,\"g\":%d,\"b\":%d}", r, g, b);
    return api_server_ws_broadcast(buffer);
}

esp_err_t api_server_ws_broadcast_wifi(bool connected, const char *ip, int rssi)
{
    char buffer[128];
    if (connected && ip != NULL)
    {
        snprintf(buffer, sizeof(buffer), "{\"type\":\"wifi\",\"connected\":true,\"ip\":\"%s\",\"rssi\":%d}", ip, rssi);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "{\"type\":\"wifi\",\"connected\":false}");
    }
    return api_server_ws_broadcast(buffer);
}
