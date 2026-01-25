# Captive Portal Implementation Guide

This document describes how to implement the captive portal functionality on the ESP32 side to work with `captive.html`.

## Overview

When the ESP32 has no WiFi credentials stored (or connection fails), it should start in Access Point (AP) mode and serve a captive portal that allows users to configure WiFi settings.

## How Captive Portal Detection Works

Operating systems automatically send HTTP requests to known URLs to check for internet connectivity:

| OS | Detection URL | Expected Response |
|---|---|---|
| **iOS/macOS** | `http://captive.apple.com/hotspot-detect.html` | `<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>` |
| **Android** | `http://connectivitycheck.gstatic.com/generate_204` | HTTP 204 No Content |
| **Windows** | `http://www.msftconnecttest.com/connecttest.txt` | `Microsoft Connect Test` |

If the response doesn't match, the OS assumes there's a captive portal and opens a browser.

## ESP32 Implementation Steps

### 1. Start Access Point Mode

```c
wifi_config_t ap_config = {
    .ap = {
        .ssid = "SystemControl-Setup",
        .ssid_len = 0,
        .password = "",  // Open network for easy access
        .max_connection = 4,
        .authmode = WIFI_AUTH_OPEN
    }
};
esp_wifi_set_mode(WIFI_MODE_AP);
esp_wifi_set_config(WIFI_IF_AP, &ap_config);
esp_wifi_start();
```

### 2. Start DNS Server (DNS Hijacking)

Redirect ALL DNS queries to the ESP32's IP address:

```c
// Simplified example - use a proper DNS server component
void dns_server_task(void *pvParameters) {
    // Listen on UDP port 53
    // For any DNS query, respond with ESP32's AP IP (e.g., 192.168.4.1)
}
```

### 3. Configure HTTP Server with Redirects

```c
// Handler for captive portal detection URLs
esp_err_t captive_redirect_handler(httpd_req_t *req) {
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/captive.html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

// Register handlers for detection URLs
httpd_uri_t apple_detect = {
    .uri = "/hotspot-detect.html",
    .method = HTTP_GET,
    .handler = captive_redirect_handler
};

httpd_uri_t android_detect = {
    .uri = "/generate_204",
    .method = HTTP_GET,
    .handler = captive_redirect_handler
};

// Catch-all for any unknown paths
httpd_uri_t catch_all = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = captive_redirect_handler
};
```

### 4. Serve Static Files

Serve the captive portal files from SPIFFS/LittleFS:

- `/captive.html` - Main captive portal page
- `/favicon.svg` - Favicon
- `/css/shared.css` - Shared styles
- `/css/captive.css` - Captive-specific styles
- `/js/i18n.js` - Internationalization
- `/js/wifi-shared.js` - WiFi configuration logic

### 5. Implement WiFi Configuration API

```c
// POST /api/wifi/config
// Body: { "ssid": "NetworkName", "password": "SecretPassword" }
esp_err_t wifi_config_handler(httpd_req_t *req) {
    // 1. Parse JSON body
    // 2. Store credentials in NVS
    // 3. Send success response
    // 4. Schedule restart/reconnect
    return ESP_OK;
}

// GET /api/wifi/scan
// Returns: [{ "ssid": "Network1", "rssi": -45 }, ...]
esp_err_t wifi_scan_handler(httpd_req_t *req) {
    // 1. Perform WiFi scan
    // 2. Return JSON array of networks
    return ESP_OK;
}
```

## Flow After User Submits WiFi Credentials

```
1. User enters SSID + Password, clicks "Connect"
   ↓
2. Frontend sends POST /api/wifi/config
   ↓
3. ESP32 stores credentials in NVS (Non-Volatile Storage)
   ↓
4. ESP32 sends HTTP 200 OK response
   ↓
5. Frontend shows countdown (10 seconds)
   ↓
6. ESP32 stops AP mode
   ↓
7. ESP32 connects to configured WiFi
   ↓
8. ESP32 gets new IP from router (e.g., 192.168.1.42)
   ↓
9. User connects phone/PC to normal WiFi
   ↓
10. User accesses ESP32 via new IP or mDNS (e.g., http://system-control.local)
```

## Recommended: mDNS Support

Register an mDNS hostname so users can access the device without knowing the IP:

```c
mdns_init();
mdns_hostname_set("system-control");
mdns_instance_name_set("System Control");
```

Then the device is accessible at: `http://system-control.local`

## Error Handling / Fallback

If WiFi connection fails after credentials are saved:

1. Wait for connection timeout (e.g., 30 seconds)
2. If connection fails, restart in AP mode
3. Show error message on captive portal
4. Allow user to re-enter credentials

```c
// Pseudo-code
if (wifi_connect_timeout()) {
    nvs_erase_key("wifi_ssid");
    nvs_erase_key("wifi_password");
    esp_restart();  // Will boot into AP mode again
}
```

## API Endpoints Summary

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/wifi/scan` | Scan for available networks |
| POST | `/api/wifi/config` | Save WiFi credentials |
| GET | `/api/wifi/status` | Get current connection status |

### Request/Response Examples

**GET /api/wifi/scan**
```json
[
    { "ssid": "HomeNetwork", "rssi": -45, "secure": true },
    { "ssid": "GuestWiFi", "rssi": -67, "secure": false }
]
```

**POST /api/wifi/config**
```json
{ "ssid": "HomeNetwork", "password": "MySecretPassword" }
```

**GET /api/wifi/status**
```json
{
    "connected": true,
    "ssid": "HomeNetwork",
    "ip": "192.168.1.42",
    "rssi": -52
}
```

## Security Considerations

1. **Open AP**: The setup AP is intentionally open for easy access. Keep setup time short.
2. **HTTPS**: Consider using HTTPS for the main interface (after WiFi setup).
3. **Timeout**: Auto-disable AP mode after successful connection.
4. **Button Reset**: Implement a physical button to reset WiFi credentials and re-enter AP mode.
