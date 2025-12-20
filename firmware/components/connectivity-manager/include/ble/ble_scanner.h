#pragma once

#include "ble_device.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void start_scan(void);
    int get_device_count(void);
    device_info_t *get_device(int index);
#ifdef __cplusplus
}
#endif
