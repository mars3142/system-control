#pragma once

#include <esp_err.h>
#include <esp_http_server.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Configuration for the API server
     */
    typedef struct
    {
        const char *hostname;  ///< mDNS hostname (default: "system-control")
        uint16_t port;         ///< HTTP server port (default: 80)
        const char *base_path; ///< Base path for static files (default: "/storage/www")
        bool enable_cors;      ///< Enable CORS headers (default: true)
    } api_server_config_t;

#ifdef CONFIG_API_SERVER_ENABLE_CORS
#define API_SERVER_ENABLE_CORS true
#else
#define API_SERVER_ENABLE_CORS false
#endif

/**
 * @brief Default configuration for the API server
 */
#define API_SERVER_CONFIG_DEFAULT()                                                                                    \
    {                                                                                                                  \
        .hostname = CONFIG_API_SERVER_HOSTNAME,                                                                        \
        .port = CONFIG_API_SERVER_PORT,                                                                                \
        .base_path = CONFIG_API_SERVER_STATIC_FILES_PATH,                                                              \
        .enable_cors = API_SERVER_ENABLE_CORS,                                                                         \
    }

    /**
     * @brief Initialize and start the API server with mDNS
     *
     * This function starts an HTTP server with:
     * - REST API endpoints
     * - WebSocket endpoint at /ws
     * - mDNS registration (system-control.local)
     * - Static file serving from SPIFFS
     *
     * @param config Pointer to server configuration, or NULL for defaults
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t api_server_start(const api_server_config_t *config);

    /**
     * @brief Stop the API server and mDNS
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t api_server_stop(void);

    /**
     * @brief Check if the API server is running
     *
     * @return true if server is running
     */
    bool api_server_is_running(void);

    /**
     * @brief Get the HTTP server handle
     *
     * @return httpd_handle_t Server handle, or NULL if not running
     */
    httpd_handle_t api_server_get_handle(void);

    /**
     * @brief Broadcast a message to all connected WebSocket clients
     *
     * @param message JSON message to broadcast
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t api_server_ws_broadcast(const char *message);

    /**
     * @brief Broadcast a status update to all WebSocket clients
     *
     * @param on Light power state
     * @param mode Current mode (day/night/simulation)
     * @param schema Active schema filename
     * @param r Red value (0-255)
     * @param g Green value (0-255)
     * @param b Blue value (0-255)
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t api_server_ws_broadcast_status(bool on, const char *mode, const char *schema, uint8_t r, uint8_t g,
                                             uint8_t b);

    /**
     * @brief Broadcast a color update to all WebSocket clients
     *
     * @param r Red value (0-255)
     * @param g Green value (0-255)
     * @param b Blue value (0-255)
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t api_server_ws_broadcast_color(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Broadcast a WiFi status update to all WebSocket clients
     *
     * @param connected Connection state
     * @param ip IP address (can be NULL if not connected)
     * @param rssi Signal strength
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t api_server_ws_broadcast_wifi(bool connected, const char *ip, int rssi);

#ifdef __cplusplus
}
#endif
