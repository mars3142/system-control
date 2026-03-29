#include "bifrost/api_handlers.h"
#include "bifrost/api_handlers_util.h"
#include "persistence_manager.h"

#include <cJSON.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <sdkconfig.h>
#include <string.h>

static const char *TAG = "api_wifi";

// ============================================================================
// Capabilities API
// ============================================================================

esp_err_t api_capabilities_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/capabilities");

    bool thread = false;
#if defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
#if defined(CONFIG_IRIS_ENABLED)
    thread = true;
#endif
#endif
    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "thread", thread);
    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    esp_err_t res = send_json_response(req, response);
    free(response);
    return res;
}

// ============================================================================
// WiFi API
// ============================================================================

esp_err_t api_wifi_scan_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/wifi/scan");

    // Start WiFi scan non-blocking (async) to avoid blocking HTTP server
    // The scan will complete in background, results available on next request
    wifi_scan_config_t scan_config = {.ssid = NULL, .bssid = NULL, .channel = 0, .show_hidden = true};
    esp_err_t err = esp_wifi_scan_start(&scan_config, false);
    if (err != ESP_OK)
    {
        ESP_LOGD(TAG, "WiFi scan start: %s (may already be scanning)", esp_err_to_name(err));
        // Continue and return cached results - don't block on error
    }

    // Get cached scan results (from previous scan if available)
    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);

    cJSON *json = cJSON_CreateArray();

    if (ap_num > 0)
    {
        wifi_ap_record_t *ap_list = heap_caps_calloc(ap_num, sizeof(wifi_ap_record_t), MALLOC_CAP_DEFAULT);
        if (ap_list)
        {
            esp_wifi_scan_get_ap_records(&ap_num, ap_list);

            for (int i = 0; i < ap_num; i++)
            {
                if (ap_list[i].ssid[0] != '\0')
                {
                    cJSON *entry = cJSON_CreateObject();
                    cJSON_AddStringToObject(entry, "ssid", (const char *)ap_list[i].ssid);
                    cJSON_AddNumberToObject(entry, "rssi", ap_list[i].rssi);
                    bool secure = ap_list[i].authmode != WIFI_AUTH_OPEN;
                    cJSON_AddBoolToObject(entry, "secure", secure);
                    cJSON_AddItemToArray(json, entry);
                }
            }
            free(ap_list);
        }
    }

    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    esp_err_t res = send_json_response(req, response);
    free(response);
    return res;
}

static void reboot_task(void *param)
{
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}

esp_err_t api_wifi_config_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "POST /api/wifi/config");
    ESP_LOGI(TAG, "Request content length: %d", req->content_len);

    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
    {
        return send_error_response(req, 400, "Failed to receive request body");
    }
    buf[ret] = '\0';

    cJSON *json = cJSON_Parse(buf);
    if (json)
    {
        cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
        cJSON *pw = cJSON_GetObjectItem(json, "password");
        if (is_valid(ssid) && is_valid(pw))
        {
            persistence_manager_t pm;
            if (persistence_manager_init(&pm, "wifi_config") == ESP_OK)
            {
                persistence_manager_set_string(&pm, "ssid", ssid->valuestring);
                persistence_manager_set_string(&pm, "password", pw->valuestring);
                persistence_manager_deinit(&pm);
            }
        }
        if (is_valid(pw))
        {
            size_t pwlen = strlen(pw->valuestring);
            char *masked = heap_caps_malloc(pwlen + 1, MALLOC_CAP_DEFAULT);
            if (masked)
            {
                memset(masked, '*', pwlen);
                masked[pwlen] = '\0';
                cJSON_ReplaceItemInObject(json, "password", cJSON_CreateString(masked));
                char *logstr = cJSON_PrintUnformatted(json);
                ESP_LOGI(TAG, "Received WiFi config: %s", logstr);
                free(logstr);
                free(masked);
            }
            else
            {
                ESP_LOGI(TAG, "Received WiFi config: %s", buf);
            }
        }
        else
        {
            ESP_LOGI(TAG, "Received WiFi config: %s", buf);
        }
        cJSON_Delete(json);
    }
    else
    {
        ESP_LOGI(TAG, "Received WiFi config: %s", buf);
    }

    // Define a reboot task function
    xTaskCreate(reboot_task, "reboot_task", 2048, NULL, 5, NULL);

    set_cors_headers(req);
    httpd_resp_set_status(req, "200 OK");
    return httpd_resp_sendstr(req, "{\"status\":\"ok\"}");
}

esp_err_t api_wifi_status_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "GET /api/wifi/status");

    wifi_ap_record_t ap_info;
    bool connected = false;
    char ssid[33] = "";
    char ip[16] = "";
    int rssi = 0;

    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
    {
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
        {
            connected = true;
            strncpy(ssid, (const char *)ap_info.ssid, sizeof(ssid) - 1);
            rssi = ap_info.rssi;
        }
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif)
        {
            esp_netif_ip_info_t ip_info;
            if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK)
            {
                snprintf(ip, sizeof(ip), "%d.%d.%d.%d", esp_ip4_addr1(&ip_info.ip), esp_ip4_addr2(&ip_info.ip),
                         esp_ip4_addr3(&ip_info.ip), esp_ip4_addr4(&ip_info.ip));
            }
        }
    }

    cJSON *json = cJSON_CreateObject();
    cJSON_AddBoolToObject(json, "connected", connected);
    cJSON_AddStringToObject(json, "ssid", ssid);
    cJSON_AddStringToObject(json, "ip", ip);
    cJSON_AddNumberToObject(json, "rssi", rssi);
    char *response = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    esp_err_t res = send_json_response(req, response);
    free(response);
    return res;
}
