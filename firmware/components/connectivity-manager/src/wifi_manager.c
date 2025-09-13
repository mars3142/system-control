#include "wifi_manager.h"

#include "esp_event.h"
#include "esp_insights.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "led_status.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

// Event group to signal when we are connected
static EventGroupHandle_t s_wifi_event_group;

// The bits for the event group
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi_manager";

static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        led_behavior_t led0_behavior = {
            .mode = LED_MODE_BLINK, .color = {.r = 0, .g = 50, .b = 0}, .on_time_ms = 200, .off_time_ms = 200};
        led_status_set_behavior(0, led0_behavior);

        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < CONFIG_WIFI_CONNECT_RETRIES)
        {
            led_behavior_t led0_behavior = {
                .mode = LED_MODE_BLINK, .color = {.r = 0, .g = 50, .b = 0}, .on_time_ms = 200, .off_time_ms = 200};
            led_status_set_behavior(0, led0_behavior);

            esp_wifi_connect();
            s_retry_num++;
            ESP_DIAG_EVENT(TAG, "Retrying to connect to the AP");
        }
        else
        {
            led_behavior_t led0_behavior = {
                .mode = LED_MODE_BLINK, .color = {.r = 0, .g = 50, .b = 0}, .on_time_ms = 1000, .off_time_ms = 500};
            led_status_set_behavior(0, led0_behavior);

            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_DIAG_EVENT(TAG, "Failed to connect to the AP");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        led_behavior_t led0_behavior = {
            .mode = LED_MODE_SOLID,
            .color = {.r = 0, .g = 50, .b = 0},
        };
        led_status_set_behavior(0, led0_behavior);

        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_DIAG_EVENT(TAG, "Got IP address:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_manager_init()
{
    s_wifi_event_group = xEventGroupCreate();

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

    wifi_config_t wifi_config = {
        .sta =
            {
                .ssid = CONFIG_WIFI_SSID,
                .password = CONFIG_WIFI_PASSWORD,
                .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_DIAG_EVENT(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or
       connection failed for the maximum number of retries (WIFI_FAIL_BIT). The bits are set by event_handler() */
    EventBits_t bits =
        xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_DIAG_EVENT(TAG, "Connected to AP SSID:%s", CONFIG_WIFI_SSID);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_DIAG_EVENT(TAG, "Failed to connect to SSID:%s", CONFIG_WIFI_SSID);
    }
    else
    {
        ESP_LOGE(TAG, "Unexpected event");
    }
}
