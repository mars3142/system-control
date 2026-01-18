
#include "persistence_manager.h"
#include <esp_log.h>
#include <string.h>

#define TAG "persistence_manager"

esp_err_t persistence_manager_factory_reset(void)
{
    // Erase the entire NVS flash (factory reset)
    esp_err_t err = nvs_flash_erase();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Factory reset failed: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t persistence_manager_init(persistence_manager_t *pm, const char *nvs_namespace)
{
    if (!pm)
        return ESP_ERR_INVALID_ARG;
    strncpy(pm->nvs_namespace, nvs_namespace ? nvs_namespace : "config", sizeof(pm->nvs_namespace) - 1);
    pm->nvs_namespace[sizeof(pm->nvs_namespace) - 1] = '\0';
    pm->initialized = false;
    esp_err_t err = nvs_open(pm->nvs_namespace, NVS_READWRITE, &pm->nvs_handle);
    if (err == ESP_OK)
    {
        pm->initialized = true;
        ESP_LOGD(TAG, "Initialized with namespace: %s", pm->nvs_namespace);
        return ESP_OK;
    }
    ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
    return err;
}

esp_err_t persistence_manager_deinit(persistence_manager_t *pm)
{
    if (pm && pm->initialized)
    {
        nvs_close(pm->nvs_handle);
        pm->initialized = false;
    }
    return ESP_OK;
}

bool persistence_manager_is_initialized(const persistence_manager_t *pm)
{
    return pm && pm->initialized;
}

bool persistence_manager_has_key(const persistence_manager_t *pm, const char *key)
{
    if (!persistence_manager_is_initialized(pm))
        return false;
    size_t required_size = 0;
    esp_err_t err = nvs_get_blob(pm->nvs_handle, key, NULL, &required_size);
    return err == ESP_OK;
}

void persistence_manager_remove_key(persistence_manager_t *pm, const char *key)
{
    if (!persistence_manager_is_initialized(pm))
        return;
    esp_err_t err = nvs_erase_key(pm->nvs_handle, key);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG, "Failed to remove key '%s': %s", key, esp_err_to_name(err));
    }
}

void persistence_manager_clear(persistence_manager_t *pm)
{
    if (!persistence_manager_is_initialized(pm))
        return;
    esp_err_t err = nvs_erase_all(pm->nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to clear all keys: %s", esp_err_to_name(err));
    }
}

size_t persistence_manager_get_key_count(const persistence_manager_t *pm)
{
    if (!persistence_manager_is_initialized(pm))
        return 0;
    nvs_iterator_t it = NULL;
    esp_err_t err = nvs_entry_find(NVS_DEFAULT_PART_NAME, pm->nvs_namespace, NVS_TYPE_ANY, &it);
    if (err != ESP_OK || it == NULL)
        return 0;
    size_t count = 0;
    while (it != NULL)
    {
        count++;
        err = nvs_entry_next(&it);
        if (err != ESP_OK)
            break;
    }
    nvs_release_iterator(it);
    return count;
}

bool persistence_manager_save(persistence_manager_t *pm)
{
    if (!persistence_manager_is_initialized(pm))
        return false;
    esp_err_t err = nvs_commit(pm->nvs_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit NVS: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool persistence_manager_load(persistence_manager_t *pm)
{
    return persistence_manager_is_initialized(pm);
}

void persistence_manager_set_bool(persistence_manager_t *pm, const char *key, bool value)
{
    if (!persistence_manager_is_initialized(pm))
        return;
    uint8_t val = value ? 1 : 0;
    esp_err_t err = nvs_set_u8(pm->nvs_handle, key, val);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set bool key '%s': %s", key, esp_err_to_name(err));
    }
}

void persistence_manager_set_int(persistence_manager_t *pm, const char *key, int32_t value)
{
    if (!persistence_manager_is_initialized(pm))
        return;
    esp_err_t err = nvs_set_i32(pm->nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set int key '%s': %s", key, esp_err_to_name(err));
    }
}

void persistence_manager_set_float(persistence_manager_t *pm, const char *key, float value)
{
    if (!persistence_manager_is_initialized(pm))
        return;
    esp_err_t err = nvs_set_blob(pm->nvs_handle, key, &value, sizeof(float));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set float key '%s': %s", key, esp_err_to_name(err));
    }
}

void persistence_manager_set_double(persistence_manager_t *pm, const char *key, double value)
{
    if (!persistence_manager_is_initialized(pm))
        return;
    esp_err_t err = nvs_set_blob(pm->nvs_handle, key, &value, sizeof(double));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set double key '%s': %s", key, esp_err_to_name(err));
    }
}

void persistence_manager_set_string(persistence_manager_t *pm, const char *key, const char *value)
{
    if (!persistence_manager_is_initialized(pm))
        return;
    esp_err_t err = nvs_set_str(pm->nvs_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set string key '%s': %s", key, esp_err_to_name(err));
    }
}

bool persistence_manager_get_bool(const persistence_manager_t *pm, const char *key, bool default_value)
{
    if (!persistence_manager_is_initialized(pm))
        return default_value;
    uint8_t value;
    esp_err_t err = nvs_get_u8(pm->nvs_handle, key, &value);
    if (err != ESP_OK)
        return default_value;
    return value != 0;
}

int32_t persistence_manager_get_int(const persistence_manager_t *pm, const char *key, int32_t default_value)
{
    if (!persistence_manager_is_initialized(pm))
        return default_value;
    int32_t value;
    esp_err_t err = nvs_get_i32(pm->nvs_handle, key, &value);
    if (err != ESP_OK)
        return default_value;
    return value;
}

float persistence_manager_get_float(const persistence_manager_t *pm, const char *key, float default_value)
{
    if (!persistence_manager_is_initialized(pm))
        return default_value;
    float value;
    size_t required_size = sizeof(float);
    esp_err_t err = nvs_get_blob(pm->nvs_handle, key, &value, &required_size);
    if (err != ESP_OK || required_size != sizeof(float))
        return default_value;
    return value;
}

double persistence_manager_get_double(const persistence_manager_t *pm, const char *key, double default_value)
{
    if (!persistence_manager_is_initialized(pm))
        return default_value;
    double value;
    size_t required_size = sizeof(double);
    esp_err_t err = nvs_get_blob(pm->nvs_handle, key, &value, &required_size);
    if (err != ESP_OK || required_size != sizeof(double))
        return default_value;
    return value;
}

void persistence_manager_get_string(const persistence_manager_t *pm, const char *key, char *out_value, size_t max_len,
                                    const char *default_value)
{
    if (!persistence_manager_is_initialized(pm))
    {
        strncpy(out_value, default_value, max_len - 1);
        out_value[max_len - 1] = '\0';
        return;
    }
    size_t required_size = 0;
    esp_err_t err = nvs_get_str(pm->nvs_handle, key, NULL, &required_size);
    if (err != ESP_OK || required_size == 0 || required_size > max_len)
    {
        strncpy(out_value, default_value, max_len - 1);
        out_value[max_len - 1] = '\0';
        return;
    }
    err = nvs_get_str(pm->nvs_handle, key, out_value, &required_size);
    if (err != ESP_OK)
    {
        strncpy(out_value, default_value, max_len - 1);
        out_value[max_len - 1] = '\0';
        return;
    }
}
