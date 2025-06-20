#pragma once

typedef enum
{
    VALUE_TYPE_STRING,
    VALUE_TYPE_INT32,
} persistence_value_t;

typedef struct
{
    void *handle;
    void (*save)(persistence_value_t value_type, const char *key, const void *value);
} persistence_t;

#ifdef __cplusplus
extern "C"
{
#endif
void *persistence_init(const char *namespace_name);
void persistence_save(persistence_value_t value_type, const char *key, const void *value);
void *persistence_load(persistence_value_t value_type, const char *key, void *out);
void persistence_deinit();
#ifdef __cplusplus
}
#endif