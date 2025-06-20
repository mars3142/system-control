#pragma once

#include "IPersistenceManager.h"
#include <string>
#include <unordered_map>

#include <nvs.h>
#include <nvs_flash.h>

/**
 * @class PersistenceManager
 * @brief ESP32-specific implementation using NVS (Non-Volatile Storage)
 * @details This implementation uses ESP32's NVS API for persistent storage
 *          in flash memory, providing a platform-optimized solution for
 *          embedded systems.
 */
class PersistenceManager : public IPersistenceManager
{
  private:
    nvs_handle_t nvs_handle_;
    std::string namespace_;
    bool initialized_;

  public:
    explicit PersistenceManager(const std::string &nvs_namespace = "config");
    ~PersistenceManager() override;

    // IPersistenceManager implementation
    bool HasKey(const std::string &key) const override;
    void RemoveKey(const std::string &key) override;
    void Clear() override;
    size_t GetKeyCount() const override;

    bool Save() override;
    bool Load() override;

    // ESP32-specific methods
    bool Initialize();
    void Deinitialize();
    bool IsInitialized() const
    {
        return initialized_;
    }

  protected:
    // Template-spezifische Implementierungen
    void SetValueImpl(const std::string &key, bool value) override;
    void SetValueImpl(const std::string &key, int value) override;
    void SetValueImpl(const std::string &key, float value) override;
    void SetValueImpl(const std::string &key, double value) override;
    void SetValueImpl(const std::string &key, const std::string &value) override;

    bool GetValueImpl(const std::string &key, bool defaultValue) const override;
    int GetValueImpl(const std::string &key, int defaultValue) const override;
    float GetValueImpl(const std::string &key, float defaultValue) const override;
    double GetValueImpl(const std::string &key, double defaultValue) const override;
    std::string GetValueImpl(const std::string &key, const std::string &defaultValue) const override;

  private:
    bool EnsureInitialized() const;
};