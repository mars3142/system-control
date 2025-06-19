#pragma once

typedef enum
{
    VALUE_TYPE_STRING,
    VALUE_TYPE_INT32,
} persistence_value_t;

typedef struct
{
    void *handle;
    void (*save)(const char *key, const char *value);
} persistence_t;

void *persistence_init(const char *namespace_name);
void persistence_save(persistence_value_t value_type, const char *key, const void *value);
void *persistence_load(persistence_value_t value_type, const char *key, void *out);
void persistence_deinit();
