#pragma once

#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Structure to manage persistent storage using NVS.
     *
     * This struct holds the NVS handle, namespace, and initialization state
     * for managing persistent key-value storage on the device.
     */
    typedef struct
    {
        /** Handle to the NVS storage. */
        nvs_handle_t nvs_handle;
        /** Namespace used for NVS operations (max 15 chars + null terminator). */
        char nvs_namespace[16];
        /** Indicates if the persistence manager is initialized. */
        bool initialized;
    } persistence_manager_t;

    /**
     * @brief Erases the entire NVS flash (factory reset).
     *
     * Warning: This will remove all stored data and namespaces!
     *
     * @return esp_err_t ESP_OK on success, otherwise error code.
     */
    esp_err_t persistence_manager_factory_reset(void);

    /**
     * @brief Initialize the persistence manager with a given NVS namespace.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param nvs_namespace Namespace to use for NVS operations.
     */
    esp_err_t persistence_manager_init(persistence_manager_t *pm, const char *nvs_namespace);

    /**
     * @brief Deinitialize the persistence manager and release resources.
     *
     * @param pm Pointer to the persistence manager structure.
     */
    esp_err_t persistence_manager_deinit(persistence_manager_t *pm);

    /**
     * @brief Check if the persistence manager is initialized.
     *
     * @param pm Pointer to the persistence manager structure.
     * @return true if initialized, false otherwise.
     */
    bool persistence_manager_is_initialized(const persistence_manager_t *pm);

    /**
     * @brief Check if a key exists in the NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to check for existence.
     * @return true if the key exists, false otherwise.
     */
    bool persistence_manager_has_key(const persistence_manager_t *pm, const char *key);

    /**
     * @brief Remove a key from the NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to remove.
     */
    void persistence_manager_remove_key(persistence_manager_t *pm, const char *key);

    /**
     * @brief Clear all keys from the NVS storage in the current namespace.
     *
     * @param pm Pointer to the persistence manager structure.
     */
    void persistence_manager_clear(persistence_manager_t *pm);

    /**
     * @brief Get the number of keys stored in the current namespace.
     *
     * @param pm Pointer to the persistence manager structure.
     * @return Number of keys.
     */
    size_t persistence_manager_get_key_count(const persistence_manager_t *pm);

    /**
     * @brief Save all pending changes to NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @return true if successful, false otherwise.
     */
    bool persistence_manager_save(persistence_manager_t *pm);

    /**
     * @brief Load all data from NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @return true if successful, false otherwise.
     */
    bool persistence_manager_load(persistence_manager_t *pm);

    /**
     * @brief Set a boolean value for a key in NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to set.
     * @param value Boolean value to store.
     */
    void persistence_manager_set_bool(persistence_manager_t *pm, const char *key, bool value);

    /**
     * @brief Set an integer value for a key in NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to set.
     * @param value Integer value to store.
     */
    void persistence_manager_set_int(persistence_manager_t *pm, const char *key, int32_t value);

    /**
     * @brief Set a float value for a key in NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to set.
     * @param value Float value to store.
     */
    void persistence_manager_set_float(persistence_manager_t *pm, const char *key, float value);

    /**
     * @brief Set a double value for a key in NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to set.
     * @param value Double value to store.
     */
    void persistence_manager_set_double(persistence_manager_t *pm, const char *key, double value);

    /**
     * @brief Set a string value for a key in NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to set.
     * @param value String value to store.
     */
    void persistence_manager_set_string(persistence_manager_t *pm, const char *key, const char *value);

    /**
     * @brief Get a boolean value for a key from NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to retrieve.
     * @param default_value Value to return if key does not exist.
     * @return Boolean value.
     */
    bool persistence_manager_get_bool(const persistence_manager_t *pm, const char *key, bool default_value);

    /**
     * @brief Get an integer value for a key from NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to retrieve.
     * @param default_value Value to return if key does not exist.
     * @return Integer value.
     */
    int32_t persistence_manager_get_int(const persistence_manager_t *pm, const char *key, int32_t default_value);

    /**
     * @brief Get a float value for a key from NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to retrieve.
     * @param default_value Value to return if key does not exist.
     * @return Float value.
     */
    float persistence_manager_get_float(const persistence_manager_t *pm, const char *key, float default_value);

    /**
     * @brief Get a double value for a key from NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to retrieve.
     * @param default_value Value to return if key does not exist.
     * @return Double value.
     */
    double persistence_manager_get_double(const persistence_manager_t *pm, const char *key, double default_value);

    /**
     * @brief Get a string value for a key from NVS storage.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to retrieve.
     * @param out_value Buffer to store the retrieved string.
     * @param max_len Maximum length of the output buffer.
     * @param default_value Value to use if key does not exist.
     */
    void persistence_manager_get_string(const persistence_manager_t *pm, const char *key, char *out_value,
                                        size_t max_len, const char *default_value);

    /**
     * @brief Set a blob (binary data) value for a key in NVS storage.
     *
     * This function stores arbitrary binary data under the given key.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to set.
     * @param value Pointer to the data to store.
     * @param length Length of the data in bytes.
     */
    void persistence_manager_set_blob(persistence_manager_t *pm, const char *key, const void *value, size_t length);

    /**
     * @brief Get a blob (binary data) value for a key from NVS storage.
     *
     * This function retrieves binary data previously stored under the given key.
     *
     * @param pm Pointer to the persistence manager structure.
     * @param key Key to retrieve.
     * @param out_value Buffer to store the retrieved data.
     * @param max_length Maximum length of the output buffer in bytes.
     * @param out_length Pointer to variable to receive the actual data length.
     * @return true if the blob was found and read successfully, false otherwise.
     */
    bool persistence_manager_get_blob(const persistence_manager_t *pm, const char *key, void *out_value,
                                      size_t max_length, size_t *out_length);

#ifdef __cplusplus
}
#endif
