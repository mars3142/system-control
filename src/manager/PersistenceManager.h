#pragma once

#include "common/IPersistenceManager.h"
#include <SDL3/SDL.h>
#include <unordered_map>
#include <variant>

class PersistenceManager final : public IPersistenceManager {
public:
    using ValueType = std::variant<
        bool,
        int,
        float,
        double,
        std::string
    >;

private:
    std::unordered_map<std::string, ValueType> data_;
    std::string filename_;

public:
    explicit PersistenceManager(std::string  filename = "settings.dat");
    ~PersistenceManager() override;

    // IPersistenceManager implementation
    bool HasKey(const std::string& key) const override;
    void RemoveKey(const std::string& key) override;
    void Clear() override;
    size_t GetKeyCount() const override { return data_.size(); }

    bool Save() override;
    bool Load() override;

    // Erweiterte SDL3-spezifische Methoden
    bool SaveToFile(const std::string& filename);
    bool LoadFromFile(const std::string& filename);

protected:
    // Template-spezifische Implementierungen
    void SetValueImpl(const std::string& key, bool value) override;
    void SetValueImpl(const std::string& key, int value) override;
    void SetValueImpl(const std::string& key, float value) override;
    void SetValueImpl(const std::string& key, double value) override;
    void SetValueImpl(const std::string& key, const std::string& value) override;

    bool GetValueImpl(const std::string& key, bool defaultValue) const override;
    int GetValueImpl(const std::string& key, int defaultValue) const override;
    float GetValueImpl(const std::string& key, float defaultValue) const override;
    double GetValueImpl(const std::string& key, double defaultValue) const override;
    std::string GetValueImpl(const std::string& key, const std::string& defaultValue) const override;

private:
    // Interne Hilfsmethoden für Serialisierung
    static bool WriteValueToStream(SDL_IOStream* stream, const ValueType& value) ;
    static bool ReadValueFromStream(SDL_IOStream* stream, ValueType& value) ;

    enum class TypeId : uint8_t {
        BOOL = 0,
        INT = 1,
        FLOAT = 2,
        DOUBLE = 3,
        STRING = 4
    };

    static TypeId GetTypeId(const ValueType& value);
};
