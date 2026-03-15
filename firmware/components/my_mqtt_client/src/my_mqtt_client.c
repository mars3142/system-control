#include "my_mqtt_client.h"
#include "esp_app_desc.h"
#include "esp_err.h"
#include "esp_interface.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "mqtt_client.h"
#include "sdkconfig.h"

#include <cJSON.h>
#include <esp_timer.h>
#include <sys/time.h>

#define DEVICE_TOPIC_MAX_LEN 60

static const char *TAG = "mqtt_client";
static esp_mqtt_client_handle_t client = NULL;

extern const uint8_t isrgrootx1_pem_start[] asm("_binary_isrgrootx1_pem_start");
extern const uint8_t isrgrootx1_pem_end[] asm("_binary_isrgrootx1_pem_end");

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        msg_id = esp_mqtt_client_subscribe(client, "topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        msg_id = esp_mqtt_client_unsubscribe(client, "topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x ", event->msg_id, (uint8_t)*event->data);
        msg_id = esp_mqtt_client_publish(client, "topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA:");
        ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
        ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle)
        {
            ESP_LOGE(TAG, "error_type: %d", event->error_handle->error_type);
            ESP_LOGE(TAG, "esp-tls error code: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGE(TAG, "tls_stack_err: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGE(TAG, "transport_sock_errno: %d", event->error_handle->esp_transport_sock_errno);
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_client_start(void)
{
    ESP_LOGI(TAG, "Starte MQTT-Client mit URI: %s", CONFIG_MQTT_CLIENT_BROKER_URL);

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    const esp_app_desc_t *app_desc = esp_app_get_description();
    char client_id[60];
    snprintf(client_id, sizeof(client_id), "%s/%02x%02x", app_desc->project_name, mac[4], mac[5]);

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_MQTT_CLIENT_BROKER_URL,
        .broker.verification.certificate = (const char *)isrgrootx1_pem_start,
        .broker.verification.certificate_len = isrgrootx1_pem_end - isrgrootx1_pem_start,
        .credentials.username = CONFIG_MQTT_CLIENT_USERNAME,
        .credentials.client_id = client_id,
        .credentials.authentication.password = CONFIG_MQTT_CLIENT_PASSWORD,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL)
    {
        ESP_LOGE(TAG, "Fehler bei esp_mqtt_client_init!");
        return;
    }
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_err_t err = esp_mqtt_client_start(client);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_mqtt_client_start fehlgeschlagen: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "MQTT-Client gestartet");
    }
}

void get_device_topic(char *topic, size_t topic_len)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    const esp_app_desc_t *app_desc = esp_app_get_description();
    snprintf(topic, topic_len, "device/%s/%02x%02x", app_desc->project_name, mac[4], mac[5]);
}

void mqtt_publish(const char *message)
{
    // Uptime in ms
    int64_t uptime_ms = esp_timer_get_time() / 1000;

    // UTC time as ISO8601
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm_utc;
    gmtime_r(&tv.tv_sec, &tm_utc);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);

    // Firmware version
    const esp_app_desc_t *app_desc = esp_app_get_description();
    const char *firmware = app_desc->version;

    // Reset reason
    esp_reset_reason_t reset_reason = esp_reset_reason();
    const char *reset_reason_str = "UNKNOWN";
    switch (reset_reason)
    {
    case ESP_RST_POWERON:
        reset_reason_str = "POWERON";
        break;
    case ESP_RST_EXT:
        reset_reason_str = "EXT";
        break;
    case ESP_RST_SW:
        reset_reason_str = "SW";
        break;
    case ESP_RST_PANIC:
        reset_reason_str = "PANIC";
        break;
    case ESP_RST_INT_WDT:
        reset_reason_str = "INT_WDT";
        break;
    case ESP_RST_TASK_WDT:
        reset_reason_str = "TASK_WDT";
        break;
    case ESP_RST_WDT:
        reset_reason_str = "WDT";
        break;
    case ESP_RST_DEEPSLEEP:
        reset_reason_str = "DEEPSLEEP";
        break;
    case ESP_RST_BROWNOUT:
        reset_reason_str = "BROWNOUT";
        break;
    case ESP_RST_SDIO:
        reset_reason_str = "SDIO";
        break;
    default:
        break;
    }

    // Create JSON object
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    cJSON *root = cJSON_CreateObject();
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    cJSON_AddStringToObject(root, "device_id", mac_str);
    cJSON_AddNumberToObject(root, "uptime", uptime_ms);
    cJSON_AddStringToObject(root, "timestamp", timestamp);
    cJSON_AddStringToObject(root, "firmware", firmware);
    cJSON_AddStringToObject(root, "reset_reason", reset_reason_str);

    // Insert message as JSON object if possible
    char topic_with_type[128];
    strncpy(topic_with_type, "", sizeof(topic_with_type));
    topic_with_type[sizeof(topic_with_type) - 1] = '\0';

    cJSON *msg_obj = cJSON_Parse(message);
    if (msg_obj)
    {
        cJSON *type_item = cJSON_DetachItemFromObject(msg_obj, "type");
        if (type_item && cJSON_IsString(type_item))
        {
            // Extend topic
            strncat(topic_with_type, type_item->valuestring, sizeof(topic_with_type) - strlen(topic_with_type) - 1);
        }
        cJSON_AddItemToObject(root, "message", msg_obj);
        cJSON_Delete(type_item); // Free memory
    }
    else
    {
        cJSON_AddStringToObject(root, "message", message);
    }

    // Publish JSON via MQTT
    char *json_str = cJSON_PrintUnformatted(root);
    mqtt_client_publish(topic_with_type, json_str, strlen(json_str), 0, true);
    cJSON_Delete(root);
    free(json_str);
}

void mqtt_client_publish(const char *topic, const char *data, size_t len, int qos, bool retain)
{
    if (client)
    {
        char base_topic[DEVICE_TOPIC_MAX_LEN];
        get_device_topic(base_topic, sizeof(base_topic));
        char full_topic[DEVICE_TOPIC_MAX_LEN + 64];
        snprintf(full_topic, sizeof(full_topic), "%s/%s", base_topic, topic);

        int msg_id = esp_mqtt_client_publish(client, full_topic, data, len, qos, retain);
        ESP_LOGV(TAG, "Publish: topic=%s, msg_id=%d, qos=%d, retain=%d, len=%d", full_topic, msg_id, qos, retain,
                 (int)len);
    }
    else
    {
        ESP_LOGW(TAG, "Publish aufgerufen, aber Client ist nicht initialisiert!");
    }
}
