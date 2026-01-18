#include "wifi_manager.h"
#include "dns_hijack.h"

#include "api_server.h"

#include <esp_event.h>
#include <esp_insights.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <led_status.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <mdns.h>
#include <nvs_flash.h>
#include <persistence_manager.h>
#include <sdkconfig.h>
#include <string.h>

// Event group to signal WiFi connection status
static EventGroupHandle_t s_wifi_event_group;

// Event group bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi_manager";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START: Connecting to AP...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGW(TAG, "WIFI_EVENT_STA_DISCONNECTED: Verbindung verloren, versuche erneut...");
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP: IP-Adresse erhalten: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        ESP_LOGW(TAG, "IP_EVENT_STA_LOST_IP: IP-Adresse verloren!");
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
}

static void wifi_create_ap()
{
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t ap_config = {.ap = {.ssid = "system-control",
                                      .ssid_len = strlen("system-control"),
                                      .password = "",
                                      .max_connection = 4,
                                      .authmode = WIFI_AUTH_OPEN}};
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Access Point 'system-control' started");
    dns_server_start("192.168.4.1");

    led_behavior_t led_behavior = {
        .color = {.red = 50, .green = 0, .blue = 0},
        .index = 0,
        .mode = LED_MODE_SOLID,
    };
    led_status_set_behavior(led_behavior);
}

void wifi_manager_init()
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Default WiFi Station
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    // Event Handler registrieren
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    // Try to load stored WiFi configuration
    persistence_manager_t pm;
    char ssid[33] = {0};
    char password[65] = {0};
    bool have_ssid = false, have_password = false;
    if (persistence_manager_init(&pm, "wifi_config") == ESP_OK)
    {
        persistence_manager_get_string(&pm, "ssid", ssid, sizeof(ssid), "");
        persistence_manager_get_string(&pm, "password", password, sizeof(password), "");
        have_ssid = strlen(ssid) > 0;
        have_password = strlen(password) > 0;
    }

    if (have_ssid && have_password)
    {
        led_behavior_t led_behavior = {
            .on_time_ms = 250,
            .off_time_ms = 100,
            .color = {.red = 50, .green = 50, .blue = 0},
            .index = 0,
            .mode = LED_MODE_BLINK,
        };
        led_status_set_behavior(led_behavior);

        ESP_LOGI(TAG, "Found WiFi configuration: SSID='%s'", ssid);
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        wifi_config_t wifi_config = {0};
        strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
        strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        int retries = 0;
        EventBits_t bits;
        do
        {
            ESP_LOGI(TAG, "Warte auf IP-Adresse (DHCP)...");
            bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE,
                                       10000 / portTICK_PERIOD_MS);
            if (bits & WIFI_CONNECTED_BIT)
            {
                led_behavior_t led_behavior = {
                    .index = 0,
                    .color = {.red = 0, .green = 50, .blue = 0},
                    .mode = LED_MODE_SOLID,
                };
                led_status_set_behavior(led_behavior);
                ESP_LOGI(TAG, "WiFi connection established successfully (mit IP)");
                break;
            }
            retries++;
        } while (!(bits & WIFI_CONNECTED_BIT) && retries < CONFIG_WIFI_CONNECT_RETRIES);

        if (!(bits & WIFI_CONNECTED_BIT))
        {
            ESP_LOGW(TAG, "WiFi connection failed (keine IP?), switching to Access Point mode");
            // AP-Netzwerkschnittstelle initialisieren, falls noch nicht geschehen
            esp_netif_create_default_wifi_ap();
            wifi_create_ap();
        }
    }
    else
    {
        // Create Access Point
        esp_netif_create_default_wifi_ap();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        wifi_create_ap();
    }

    // API server start
    api_server_config_t s_config = API_SERVER_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(api_server_start(&s_config));
}
