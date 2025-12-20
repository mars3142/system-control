#pragma once

#include "ble_device.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void ble_connect(device_info_t *device);
    void ble_clear_bonds(void);
    void ble_clear_bond(const ble_addr_t *addr);
    void read_characteristic(uint16_t char_val_handle);
#ifdef __cplusplus
}
#endif