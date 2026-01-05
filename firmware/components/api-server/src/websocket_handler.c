#include "websocket_handler.h"

#include <esp_http_server.h>
#include <esp_log.h>
#include <string.h>

static const char *TAG = "websocket_handler";

// Store connected WebSocket client file descriptors
static int ws_clients[WS_MAX_CLIENTS];
static int ws_client_count = 0;

static void ws_clients_init(void)
{
    for (int i = 0; i < WS_MAX_CLIENTS; i++)
        ws_clients[i] = -1;
}

// Add a client to the list
static void add_client(int fd)
{
    for (int i = 0; i < WS_MAX_CLIENTS; i++)
    {
        if (ws_clients[i] == -1)
        {
            ws_clients[i] = fd;
            ws_client_count++;
            ESP_LOGI(TAG, "WebSocket client connected: fd=%d (total: %d)", fd, ws_client_count);
            return;
        }
    }
    ESP_LOGW(TAG, "Max WebSocket clients reached, cannot add fd=%d", fd);
}

// Remove a client from the list
static void remove_client(int fd)
{
    for (int i = 0; i < WS_MAX_CLIENTS; i++)
    {
        if (ws_clients[i] == fd)
        {
            ws_clients[i] = -1;
            ws_client_count--;
            ESP_LOGI(TAG, "WebSocket client disconnected: fd=%d (total: %d)", fd, ws_client_count);
            return;
        }
    }
}

// Handle incoming WebSocket message
static esp_err_t handle_ws_message(httpd_req_t *req, httpd_ws_frame_t *ws_pkt)
{
    ESP_LOGI(TAG, "Received WS message: %s", (char *)ws_pkt->payload);

    // Parse the message and handle different types
    // For now, we just check if it's a status request
    if (ws_pkt->payload != NULL && strstr((char *)ws_pkt->payload, "getStatus") != NULL)
    {
        // Send status response
        // TODO: Get actual status values
        const char *response = "{"
                               "\"type\":\"status\","
                               "\"on\":true,"
                               "\"mode\":\"simulation\","
                               "\"schema\":\"schema_01.csv\","
                               "\"color\":{\"r\":255,\"g\":240,\"b\":220}"
                               "}";

        httpd_ws_frame_t ws_resp = {.final = true,
                                    .fragmented = false,
                                    .type = HTTPD_WS_TYPE_TEXT,
                                    .payload = (uint8_t *)response,
                                    .len = strlen(response)};

        return httpd_ws_send_frame(req, &ws_resp);
    }

    return ESP_OK;
}

esp_err_t websocket_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        // This is the handshake
        ESP_LOGI(TAG, "WebSocket handshake");
        return ESP_OK;
    }

    // Receive the frame
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    // Get frame length first
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len: %s", esp_err_to_name(ret));
        return ret;
    }

    if (ws_pkt.len > 0)
    {
        // Allocate buffer for payload
        ws_pkt.payload = malloc(ws_pkt.len + 1);
        if (ws_pkt.payload == NULL)
        {
            ESP_LOGE(TAG, "Failed to allocate memory for WS payload");
            return ESP_ERR_NO_MEM;
        }

        // Receive the payload
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed: %s", esp_err_to_name(ret));
            free(ws_pkt.payload);
            return ret;
        }

        // Null-terminate the payload
        ws_pkt.payload[ws_pkt.len] = '\0';

        // Handle the message
        ret = handle_ws_message(req, &ws_pkt);

        free(ws_pkt.payload);
        return ret;
    }

    // Handle close frame
    if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE)
    {
        int fd = httpd_req_to_sockfd(req);
        remove_client(fd);
    }

    return ESP_OK;
}

// Async send structure
typedef struct
{
    httpd_handle_t hd;
    int fd;
    char *message;
} ws_async_arg_t;

// Async send work function
static void ws_async_send(void *arg)
{
    ws_async_arg_t *async_arg = (ws_async_arg_t *)arg;

    httpd_ws_frame_t ws_pkt = {.final = true,
                               .fragmented = false,
                               .type = HTTPD_WS_TYPE_TEXT,
                               .payload = (uint8_t *)async_arg->message,
                               .len = strlen(async_arg->message)};

    esp_err_t ret = httpd_ws_send_frame_async(async_arg->hd, async_arg->fd, &ws_pkt);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to send WS frame to fd=%d: %s", async_arg->fd, esp_err_to_name(ret));
        // Remove client on error
        remove_client(async_arg->fd);
    }

    free(async_arg->message);
    free(async_arg);
}

esp_err_t websocket_handler_init(httpd_handle_t server)
{
    ws_clients_init();
    // Register WebSocket URI handler
    httpd_uri_t ws_uri = {.uri = "/ws",
                          .method = HTTP_GET,
                          .handler = websocket_handler,
                          .is_websocket = true,
                          .handle_ws_control_frames = true,
                          .user_ctx = NULL};

    esp_err_t ret = httpd_register_uri_handler(server, &ws_uri);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register WebSocket handler: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "WebSocket handler initialized at /ws");
    return ESP_OK;
}

esp_err_t websocket_send(httpd_handle_t server, int fd, const char *message)
{
    if (server == NULL || message == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    ws_async_arg_t *arg = malloc(sizeof(ws_async_arg_t));
    if (arg == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    arg->hd = server;
    arg->fd = fd;
    arg->message = strdup(message);
    if (arg->message == NULL)
    {
        free(arg);
        return ESP_ERR_NO_MEM;
    }

    esp_err_t ret = httpd_queue_work(server, ws_async_send, arg);
    if (ret != ESP_OK)
    {
        free(arg->message);
        free(arg);
    }

    return ret;
}

esp_err_t websocket_broadcast(httpd_handle_t server, const char *message)
{
    if (server == NULL || message == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = ESP_OK;

    for (int i = 0; i < WS_MAX_CLIENTS; i++)
    {
        if (ws_clients[i] != -1)
        {
            esp_err_t send_ret = websocket_send(server, ws_clients[i], message);
            if (send_ret != ESP_OK)
            {
                ESP_LOGW(TAG, "Failed to queue WS message for fd=%d", ws_clients[i]);
                ret = send_ret; // Return last error
            }
        }
    }

    return ret;
}

int websocket_get_client_count(void)
{
    return ws_client_count;
}
