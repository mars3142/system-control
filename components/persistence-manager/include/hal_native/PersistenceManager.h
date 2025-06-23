#pragma once

#include "../IPersistenceManager.h"
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
    std::unordered_map<std::string, ValueType> m_data;
    std::string m_filename;

public:
    explicit PersistenceManager(std::string  filename = "settings.dat");
    ~PersistenceManager() override;

    [[nodiscard]] bool HasKey(const std::string& key) const override;
    void RemoveKey(const std::string& key) override;
    void Clear() override;
    [[nodiscard]] size_t GetKeyCount() const override { return m_data.size(); }

    bool Save() override;
    bool Load() override;

    bool SaveToFile(const std::string& filename);
    bool LoadFromFile(const std::string& filename);

protected:
    void SetValueImpl(const std::string& key, bool value) override;
    void SetValueImpl(const std::string& key, int value) override;
    void SetValueImpl(const std::string& key, float value) override;
    void SetValueImpl(const std::string& key, double value) override;
    void SetValueImpl(const std::string& key, const std::string& value) override;

    [[nodiscard]] bool GetValueImpl(const std::string& key, bool defaultValue) const override;
    [[nodiscard]] int GetValueImpl(const std::string& key, int defaultValue) const override;
    [[nodiscard]] float GetValueImpl(const std::string& key, float defaultValue) const override;
    [[nodiscard]] double GetValueImpl(const std::string& key, double defaultValue) const override;
    [[nodiscard]] std::string GetValueImpl(const std::string& key, const std::string& defaultValue) const override;

private:
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
