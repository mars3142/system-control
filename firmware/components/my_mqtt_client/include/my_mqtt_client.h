#pragma once
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void mqtt_client_start(void);
void mqtt_client_publish(const char *topic, const char *data, size_t len, int qos, bool retain);

#ifdef __cplusplus
}
#endif
