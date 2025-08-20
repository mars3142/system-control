#pragma once

#include <string>
#include <type_traits>

/**
 * @interface IPersistenceManager
 * @brief Abstract interface for platform-independent persistence management
 * @details This interface defines the contract for key-value storage and retrieval
 *          systems across different platforms (Desktop/SDL3 and ESP32).
 */
class IPersistenceManager
{
public:
    virtual ~IPersistenceManager() = default;

    /**
     * @brief Template methods for type-safe setting and retrieving of values
     * @tparam T The type of value to set (must be one of: bool, int, float, double, std::string)
     * @param key The key to associate with the value
     * @param value The value to store
     */
    template<typename T>
    void SetValue(const std::string& key, const T& value) {
        static_assert(std::is_same_v<T, bool> || 
                     std::is_same_v<T, int> || 
                     std::is_same_v<T, float> || 
                     std::is_same_v<T, double> || 
                     std::is_same_v<T, std::string>,
                     "Unsupported type for IPersistenceManager");
        SetValueImpl(key, value);
    }

    /**
     * @brief Template method for type-safe retrieval of values
     * @tparam T The type of value to retrieve
     * @param key The key to look up
     * @param defaultValue The default value to return if the key is not found
     * @return The stored value or default value if the key doesn't exist
     */
    template<typename T>
    [[nodiscard]] T GetValue(const std::string& key, const T& defaultValue = T{}) const {
        return GetValueImpl<T>(key, defaultValue);
    }

    /**
     * @brief Utility methods for key management
     */
    [[nodiscard]] virtual bool HasKey(const std::string& key) const = 0; ///< Check if a key exists
    virtual void RemoveKey(const std::string& key) = 0; ///< Remove a key-value pair
    virtual void Clear() = 0; ///< Clear all stored data
    [[nodiscard]] virtual size_t GetKeyCount() const = 0; ///< Get the number of stored keys

    /**
     * @brief Persistence operations
     */
    virtual bool Save() = 0; ///< Save data to persistent storage
    virtual bool Load() = 0; ///< Load data from persistent storage

protected:
    /**
     * @brief Template-specific implementations that must be overridden by derived classes
     * @details These methods handle the actual storage and retrieval of different data types
     */
    virtual void SetValueImpl(const std::string& key, bool value) = 0;
    virtual void SetValueImpl(const std::string& key, int value) = 0;
    virtual void SetValueImpl(const std::string& key, float value) = 0;
    virtual void SetValueImpl(const std::string& key, double value) = 0;
    virtual void SetValueImpl(const std::string& key, const std::string& value) = 0;

    [[nodiscard]] virtual bool GetValueImpl(const std::string& key, bool defaultValue) const = 0;
    [[nodiscard]] virtual int GetValueImpl(const std::string& key, int defaultValue) const = 0;
    [[nodiscard]] virtual float GetValueImpl(const std::string& key, float defaultValue) const = 0;
    [[nodiscard]] virtual double GetValueImpl(const std::string& key, double defaultValue) const = 0;
    [[nodiscard]] virtual std::string GetValueImpl(const std::string& key, const std::string& defaultValue) const = 0;

private:
    /**
     * @brief Template dispatch methods for type-safe value retrieval
     * @tparam T The type to retrieve
     * @param key The key to look up
     * @param defaultValue The default value to return
     * @return The retrieved value or default if not found
     */
    template<typename T>
    [[nodiscard]] T GetValueImpl(const std::string& key, const T& defaultValue) const
    {
        if constexpr (std::is_same_v<T, bool>) {
            return GetValueImpl(key, static_cast<bool>(defaultValue));
        } else if constexpr (std::is_same_v<T, int>) {
            return GetValueImpl(key, static_cast<int>(defaultValue));
        } else if constexpr (std::is_same_v<T, float>) {
            return GetValueImpl(key, static_cast<float>(defaultValue));
        } else if constexpr (std::is_same_v<T, double>) {
            return GetValueImpl(key, static_cast<double>(defaultValue));
        } else if constexpr (std::is_same_v<T, std::string>) {
            return GetValueImpl(key, static_cast<const std::string&>(defaultValue));
        } else {
            static_assert(std::is_same_v<T, bool> || 
                         std::is_same_v<T, int> || 
                         std::is_same_v<T, float> || 
                         std::is_same_v<T, double> || 
                         std::is_same_v<T, std::string>,
                         "Unsupported type for IPersistenceManager");
            return defaultValue; // This line will never be reached, but satisfies the compiler
        }
    }
};