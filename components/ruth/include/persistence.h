#pragma once

typedef struct
{
    char *name;

    void (*save)(const char *key, const char *value);
} persistence_t;