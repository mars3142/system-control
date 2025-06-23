#include "hal_esp32/PersistenceManager.h"
#include <cstring>
#include <esp_log.h>

static const char *TAG = "PersistenceManager";

PersistenceManager::PersistenceManager(const std::string &nvs_namespace)
    : namespace_(nvs_namespace), initialized_(false)
{
    Initialize();
}

PersistenceManager::~PersistenceManager()
{
    Deinitialize();
}

bool PersistenceManager::Initialize()
{
    if (initialized_)
    {
        return true;
    }

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize NVS flash: %s", esp_err_to_name(err));
        return false;
    }

    // Open NVS handle
    err = nvs_open(namespace_.c_str(), NVS_READWRITE, &nvs_handle_);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
        return false;
    }

    initialized_ = true;
    ESP_LOGI(TAG, "PersistenceManager initialized with namespace: %s", namespace_.c_str());
    return true;
}

void PersistenceManager::Deinitialize()
{
    if (initialized_)
    {
        nvs_close(nvs_handle_);
        initialized_ = false;
    }
}

bool PersistenceManager::EnsureInitialized() const
{
    if (!initialized_)
    {
        ESP_LOGE(TAG, "PersistenceManager not initialized");
        return false;
    }
    return true;
}

bool PersistenceManager::HasKey(const std::string &key) const
{
    if (!EnsureInitialized())
        return false;

    size_t required_size = 0;
    esp_err_t err = nvs_get_blob(nvs_handle_, key.c_str(), nullptr, &required_size);
    return err == ESP_OK;
}

void PersistenceManager::RemoveKey(const std::string &key)
{
    if (!EnsureInitialized())
        return;

    esp_err_t err = nvs_erase_key(nvs_handle_, key.c_str());
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGE(TAG, "Failed to remove key '%s': %s", key.c_str(), esp_err_to_name(err));
    }
}

void PersistenceManager::Clear()
{
    if (!EnsureInitialized())
        return;

    esp_err_t err = nvs_erase_all(nvs_handle_);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to clear all keys: %s", esp_err_to_name(err));
    }
}

size_t PersistenceManager::GetKeyCount() const
{
    if (!EnsureInitialized())
        return 0;

    nvs_iterator_t it = nullptr;
    esp_err_t err = nvs_entry_find(NVS_DEFAULT_PART_NAME, namespace_.c_str(), NVS_TYPE_ANY, &it);

    if (err != ESP_OK || it == nullptr)
    {
        return 0;
    }

    size_t count = 0;

    while (it != nullptr)
    {
        count++;
        err = nvs_entry_next(&it);
        if (err != ESP_OK)
        {
            break;
        }
    }

    nvs_release_iterator(it);
    return count;
}

bool PersistenceManager::Save()
{
    if (!EnsureInitialized())
        return false;

    esp_err_t err = nvs_commit(nvs_handle_);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit NVS: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

bool PersistenceManager::Load()
{
    return EnsureInitialized();
}

void PersistenceManager::SetValueImpl(const std::string &key, bool value)
{
    if (!EnsureInitialized())
        return;

    uint8_t val = value ? 1 : 0;
    esp_err_t err = nvs_set_u8(nvs_handle_, key.c_str(), val);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set bool key '%s': %s", key.c_str(), esp_err_to_name(err));
    }
}

void PersistenceManager::SetValueImpl(const std::string &key, int value)
{
    if (!EnsureInitialized())
        return;

    esp_err_t err = nvs_set_i32(nvs_handle_, key.c_str(), value);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set int key '%s': %s", key.c_str(), esp_err_to_name(err));
    }
}

void PersistenceManager::SetValueImpl(const std::string &key, float value)
{
    if (!EnsureInitialized())
        return;

    esp_err_t err = nvs_set_blob(nvs_handle_, key.c_str(), &value, sizeof(float));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set float key '%s': %s", key.c_str(), esp_err_to_name(err));
    }
}

void PersistenceManager::SetValueImpl(const std::string &key, double value)
{
    if (!EnsureInitialized())
        return;

    esp_err_t err = nvs_set_blob(nvs_handle_, key.c_str(), &value, sizeof(double));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set double key '%s': %s", key.c_str(), esp_err_to_name(err));
    }
}

void PersistenceManager::SetValueImpl(const std::string &key, const std::string &value)
{
    if (!EnsureInitialized())
        return;

    esp_err_t err = nvs_set_str(nvs_handle_, key.c_str(), value.c_str());
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set string key '%s': %s", key.c_str(), esp_err_to_name(err));
    }
}

bool PersistenceManager::GetValueImpl(const std::string &key, bool defaultValue) const
{
    if (!EnsureInitialized())
        return defaultValue;

    uint8_t value;
    esp_err_t err = nvs_get_u8(nvs_handle_, key.c_str(), &value);
    if (err != ESP_OK)
    {
        return defaultValue;
    }
    return value != 0;
}

int PersistenceManager::GetValueImpl(const std::string &key, int defaultValue) const
{
    if (!EnsureInitialized())
        return defaultValue;

    int32_t value;
    esp_err_t err = nvs_get_i32(nvs_handle_, key.c_str(), &value);
    if (err != ESP_OK)
    {
        return defaultValue;
    }
    return static_cast<int>(value);
}

float PersistenceManager::GetValueImpl(const std::string &key, float defaultValue) const
{
    if (!EnsureInitialized())
        return defaultValue;

    float value;
    size_t required_size = sizeof(float);
    esp_err_t err = nvs_get_blob(nvs_handle_, key.c_str(), &value, &required_size);
    if (err != ESP_OK || required_size != sizeof(float))
    {
        return defaultValue;
    }
    return value;
}

double PersistenceManager::GetValueImpl(const std::string &key, double defaultValue) const
{
    if (!EnsureInitialized())
        return defaultValue;

    double value;
    size_t required_size = sizeof(double);
    esp_err_t err = nvs_get_blob(nvs_handle_, key.c_str(), &value, &required_size);
    if (err != ESP_OK || required_size != sizeof(double))
    {
        return defaultValue;
    }
    return value;
}

std::string PersistenceManager::GetValueImpl(const std::string &key, const std::string &defaultValue) const
{
    if (!EnsureInitialized())
        return defaultValue;

    size_t required_size = 0;
    esp_err_t err = nvs_get_str(nvs_handle_, key.c_str(), nullptr, &required_size);
    if (err != ESP_OK)
    {
        return defaultValue;
    }

    std::string value(required_size - 1, '\0'); // -1 for null terminator
    err = nvs_get_str(nvs_handle_, key.c_str(), value.data(), &required_size);
    if (err != ESP_OK)
    {
        return defaultValue;
    }

    return value;
}
