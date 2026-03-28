#include "skuld/skuld.h"

#include <cJSON.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <nvs.h>
#include <sdkconfig.h>
#include <string.h>
#include <time.h>

#define SKULD_TZ_API_URL    "https://api.firmware-hq.dev/v1/timezone"
#define SKULD_NVS_NAMESPACE "skuld"
#define SKULD_NVS_KEY_TZ    "tz_posix"
#define SKULD_DEFAULT_TZ    "CET-1CEST,M3.5.0,M10.5.0/3"
#define SKULD_TZ_MAX_LEN    64

static const char *TAG = "skuld";

extern const char isrgrootx1_pem_start[] asm("_binary_isrgrootx1_pem_start");

static void on_time_synced(struct timeval *tv)
{
    struct tm timeinfo;
    char buf[32];
    localtime_r(&tv->tv_sec, &timeinfo);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
    ESP_LOGI(TAG, "Time synchronized: %s", buf);
}

static void nvs_save_tz(const char *tz_posix)
{
    nvs_handle_t handle;
    if (nvs_open(SKULD_NVS_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) {
        return;
    }
    nvs_set_str(handle, SKULD_NVS_KEY_TZ, tz_posix);
    nvs_commit(handle);
    nvs_close(handle);
}

static bool nvs_load_tz(char *buf, size_t buf_len)
{
    nvs_handle_t handle;
    if (nvs_open(SKULD_NVS_NAMESPACE, NVS_READONLY, &handle) != ESP_OK) {
        return false;
    }
    esp_err_t err = nvs_get_str(handle, SKULD_NVS_KEY_TZ, buf, &buf_len);
    nvs_close(handle);
    return err == ESP_OK;
}

static bool fetch_tz_from_api(char *out_buf, size_t out_len)
{
    static char response[256];
    int response_len = 0;

    esp_http_client_config_t config = {
        .url = SKULD_TZ_API_URL,
        .cert_pem = isrgrootx1_pem_start,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        return false;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "HTTP open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }

    esp_http_client_fetch_headers(client);

    if (esp_http_client_get_status_code(client) != 200) {
        ESP_LOGW(TAG, "Timezone API returned HTTP %d", esp_http_client_get_status_code(client));
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }

    response_len = esp_http_client_read(client, response, sizeof(response) - 1);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (response_len <= 0) {
        return false;
    }
    response[response_len] = '\0';

    cJSON *json = cJSON_Parse(response);
    if (!json) {
        ESP_LOGW(TAG, "Failed to parse JSON response");
        return false;
    }

    bool success = false;
    const cJSON *posix_tz = cJSON_GetObjectItemCaseSensitive(json, "posix_tz");
    if (cJSON_IsString(posix_tz) && posix_tz->valuestring) {
        strncpy(out_buf, posix_tz->valuestring, out_len - 1);
        out_buf[out_len - 1] = '\0';
        success = true;
    } else {
        ESP_LOGW(TAG, "posix_tz not found in response");
    }

    cJSON_Delete(json);
    return success;
}

static void apply_timezone(const char *tz_posix)
{
    setenv("TZ", tz_posix, 1);
    tzset();
    ESP_LOGI(TAG, "Timezone set: %s", tz_posix);
}

void skuld_init(void)
{
    char tz_buf[SKULD_TZ_MAX_LEN] = {0};

    if (fetch_tz_from_api(tz_buf, sizeof(tz_buf))) {
        nvs_save_tz(tz_buf);
        ESP_LOGI(TAG, "Timezone from API: %s", tz_buf);
    } else if (nvs_load_tz(tz_buf, sizeof(tz_buf))) {
        ESP_LOGI(TAG, "Timezone from NVS: %s", tz_buf);
    } else {
        strncpy(tz_buf, SKULD_DEFAULT_TZ, sizeof(tz_buf) - 1);
        ESP_LOGW(TAG, "Timezone fallback: %s", tz_buf);
    }

    apply_timezone(tz_buf);

    if (esp_sntp_enabled()) {
        ESP_LOGI(TAG, "SNTP already initialized.");
        return;
    }

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    sntp_set_sync_interval(CONFIG_LWIP_SNTP_UPDATE_DELAY);
    sntp_set_time_sync_notification_cb(on_time_synced);
    esp_sntp_init();

    ESP_LOGI(TAG, "SNTP initialized, re-sync every %d min.", CONFIG_LWIP_SNTP_UPDATE_DELAY / 60000);
}
