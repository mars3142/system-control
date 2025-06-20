#include "persistence.h"
#include "stddef.h"
#include <stdio.h>

void *persistence_init(const char *namespace_name)
{
    return NULL;
}

void persistence_save(const persistence_value_t value_type, const char *key, const void *value)
{
    printf("Key: %s - ", key);
    switch (value_type)
    {
    case VALUE_TYPE_STRING:
        printf("Value (s): %s\n", (char *)value);
        break;

    case VALUE_TYPE_INT32:
        printf("Value (i): %d\n", *(int32_t *)value);
        break;
    }
}

void *persistence_load(persistence_value_t value_type, const char *key, void *out)
{
    return NULL;
}

void persistence_deinit()
{
}