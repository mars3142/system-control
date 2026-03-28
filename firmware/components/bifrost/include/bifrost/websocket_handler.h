#pragma once

#include <esp_http_server.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Maximum number of concurrent WebSocket connections
 */
#define WS_MAX_CLIENTS CONFIG_API_SERVER_MAX_WS_CLIENTS

    /**
     * @brief Initialize WebSocket handler
     *
     * @param server HTTP server handle
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t websocket_handler_init(httpd_handle_t server);

    /**
     * @brief WebSocket request handler
     *
     * @param req HTTP request
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t websocket_handler(httpd_req_t *req);

    /**
     * @brief Send message to a specific WebSocket client
     *
     * @param server HTTP server handle
     * @param fd Socket file descriptor
     * @param message Message to send
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t websocket_send(httpd_handle_t server, int fd, const char *message);

    /**
     * @brief Broadcast message to all connected WebSocket clients
     *
     * @param server HTTP server handle
     * @param message Message to broadcast
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t websocket_broadcast(httpd_handle_t server, const char *message);

    /**
     * @brief Get number of connected WebSocket clients
     *
     * @return int Number of connected clients
     */
    int websocket_get_client_count(void);

#ifdef __cplusplus
}
#endif
