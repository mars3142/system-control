#include "my_mqtt_client.h"
#include "esp_app_desc.h"
#include "esp_err.h"
#include "esp_interface.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "mqtt_client.h"
#include "sdkconfig.h"

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

void mqtt_client_publish(const char *topic, const char *data, size_t len, int qos, bool retain)
{
    if (client)
    {
        int msg_id = esp_mqtt_client_publish(client, topic, data, len, qos, retain);
        ESP_LOGI(TAG, "Publish: topic=%s, msg_id=%d, qos=%d, retain=%d, len=%d", topic, msg_id, qos, retain, (int)len);
    }
    else
    {
        ESP_LOGW(TAG, "Publish aufgerufen, aber Client ist nicht initialisiert!");
    }
}
