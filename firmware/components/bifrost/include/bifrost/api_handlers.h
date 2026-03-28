#pragma once

#include <esp_http_server.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Register all API handlers with the HTTP server
     *
     * @param server HTTP server handle
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t api_handlers_register(httpd_handle_t server);

    // Capabilities API
    esp_err_t api_capabilities_get_handler(httpd_req_t *req);

    // WiFi API
    esp_err_t api_wifi_scan_handler(httpd_req_t *req);
    esp_err_t api_wifi_config_handler(httpd_req_t *req);
    esp_err_t api_wifi_status_handler(httpd_req_t *req);

    // Light Control API
    esp_err_t api_light_power_handler(httpd_req_t *req);
    esp_err_t api_light_thunder_handler(httpd_req_t *req);
    esp_err_t api_light_mode_handler(httpd_req_t *req);
    esp_err_t api_light_schema_handler(httpd_req_t *req);
    esp_err_t api_light_status_handler(httpd_req_t *req);

    // LED Configuration API
    esp_err_t api_wled_config_get_handler(httpd_req_t *req);
    esp_err_t api_wled_config_post_handler(httpd_req_t *req);

    // Schema API
    esp_err_t api_schema_get_handler(httpd_req_t *req);
    esp_err_t api_schema_post_handler(httpd_req_t *req);

    // Devices API (Matter)
    esp_err_t api_devices_scan_handler(httpd_req_t *req);
    esp_err_t api_devices_pair_handler(httpd_req_t *req);
    esp_err_t api_devices_paired_handler(httpd_req_t *req);
    esp_err_t api_devices_update_handler(httpd_req_t *req);
    esp_err_t api_devices_unpair_handler(httpd_req_t *req);
    esp_err_t api_devices_toggle_handler(httpd_req_t *req);

    // Scenes API
    esp_err_t api_scenes_get_handler(httpd_req_t *req);
    esp_err_t api_scenes_post_handler(httpd_req_t *req);
    esp_err_t api_scenes_delete_handler(httpd_req_t *req);
    esp_err_t api_scenes_activate_handler(httpd_req_t *req);

    // Static file serving
    esp_err_t api_static_file_handler(httpd_req_t *req);

    // Captive portal detection
    esp_err_t api_captive_portal_handler(httpd_req_t *req);

#ifdef __cplusplus
}
#endif
