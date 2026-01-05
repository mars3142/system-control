#include "wifi_manager.h"

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
#include <nvs_flash.h>
#include <sdkconfig.h>
#include <string.h>

// Event group to signal when we are connected
static EventGroupHandle_t s_wifi_event_group;

// The bits for the event group
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi_manager";

static int s_retry_num = 0;
static int s_current_network_index = 0;

// WiFi network configuration structure
typedef struct
{
    const char *ssid;
    const char *password;
} wifi_network_config_t;

// Array of configured WiFi networks
static const wifi_network_config_t s_wifi_networks[] = {
#if CONFIG_WIFI_ENABLED
    {CONFIG_WIFI_SSID_1, CONFIG_WIFI_PASSWORD_1},
#if CONFIG_WIFI_NETWORK_COUNT >= 2
    {CONFIG_WIFI_SSID_2, CONFIG_WIFI_PASSWORD_2},
#endif
#if CONFIG_WIFI_NETWORK_COUNT >= 3
    {CONFIG_WIFI_SSID_3, CONFIG_WIFI_PASSWORD_3},
#endif
#if CONFIG_WIFI_NETWORK_COUNT >= 4
    {CONFIG_WIFI_SSID_4, CONFIG_WIFI_PASSWORD_4},
#endif
#if CONFIG_WIFI_NETWORK_COUNT >= 5
    {CONFIG_WIFI_SSID_5, CONFIG_WIFI_PASSWORD_5},
#endif
#endif
};

static const int s_wifi_network_count = sizeof(s_wifi_networks) / sizeof(s_wifi_networks[0]);

static void try_next_network(void);

static void connect_to_network(int index)
{
#if CONFIG_WIFI_ENABLED
    if (index >= s_wifi_network_count)
    {
        ESP_LOGE(TAG, "No more networks to try");
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        return;
    }

    const wifi_network_config_t *network = &s_wifi_networks[index];

    // Skip empty SSIDs
    if (network->ssid == NULL || strlen(network->ssid) == 0)
    {
        ESP_LOGW(TAG, "Skipping empty SSID at index %d", index);
        s_current_network_index++;
        s_retry_num = 0;
        try_next_network();
        return;
    }

    ESP_DIAG_EVENT(TAG, "Trying to connect to network %d: %s", index + 1, network->ssid);

    wifi_config_t wifi_config = {
        .sta =
            {
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };

    strncpy((char *)wifi_config.sta.ssid, network->ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, network->password, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_wifi_connect();
#endif
}

static void try_next_network(void)
{
#if CONFIG_WIFI_ENABLED
    s_current_network_index++;
    s_retry_num = 0;

    if (s_current_network_index < s_wifi_network_count)
    {
        connect_to_network(s_current_network_index);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to connect to any configured network");
        led_behavior_t led0_behavior = {
            .index = 0,
            .mode = LED_MODE_BLINK,
            .color = {.red = 50, .green = 0, .blue = 0},
            .on_time_ms = 1000,
            .off_time_ms = 500,
        };
        led_status_set_behavior(led0_behavior);
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
#endif
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
#if CONFIG_WIFI_ENABLED
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        led_behavior_t led0_behavior = {
            .index = 0,
            .mode = LED_MODE_BLINK,
            .color = {.red = 50, .green = 50, .blue = 0},
            .on_time_ms = 200,
            .off_time_ms = 200,
        };
        led_status_set_behavior(led0_behavior);

        connect_to_network(s_current_network_index);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < CONFIG_WIFI_CONNECT_RETRIES)
        {
            led_behavior_t led0_behavior = {
                .index = 0,
                .mode = LED_MODE_BLINK,
                .color = {.red = 50, .green = 50, .blue = 0},
                .on_time_ms = 200,
                .off_time_ms = 200,
            };
            led_status_set_behavior(led0_behavior);

            s_retry_num++;
            ESP_DIAG_EVENT(TAG, "Retrying network %d (%d/%d)", s_current_network_index + 1, s_retry_num,
                           CONFIG_WIFI_CONNECT_RETRIES);
            esp_wifi_connect();
            return;
        }

        // Retries exhausted for current network, try next one
        ESP_LOGW(TAG, "Failed to connect to network %d after %d retries, trying next...", s_current_network_index + 1,
                 CONFIG_WIFI_CONNECT_RETRIES);
        try_next_network();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        led_behavior_t led0_behavior = {
            .index = 0,
            .mode = LED_MODE_SOLID,
            .color = {.red = 0, .green = 50, .blue = 0},
        };
        led_status_set_behavior(led0_behavior);

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_DIAG_EVENT(TAG, "Got IP address:" IPSTR " (network %d: %s)", IP2STR(&event->ip_info.ip),
                       s_current_network_index + 1, s_wifi_networks[s_current_network_index].ssid);
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
#endif
}

void wifi_manager_init()
{
#if CONFIG_WIFI_ENABLED
    s_wifi_event_group = xEventGroupCreate();
    s_current_network_index = 0;
    s_retry_num = 0;

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_DIAG_EVENT(TAG, "WiFi manager initialized with %d network(s), waiting for connection...", s_wifi_network_count);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or
       connection failed for all networks (WIFI_FAIL_BIT). The bits are set by event_handler() */
    EventBits_t bits =
        xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_DIAG_EVENT(TAG, "Connected to AP SSID:%s", s_wifi_networks[s_current_network_index].ssid);

        api_server_config_t s_config = API_SERVER_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(api_server_start(&s_config));
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGE(TAG, "Failed to connect to any configured WiFi network");
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event");
    }
#endif
}
