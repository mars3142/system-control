#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    void ble_manager_task(void *pvParameter);
    void ble_connect_to_device(int index);
#ifdef __cplusplus
}
#endif
