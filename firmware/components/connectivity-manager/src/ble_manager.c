#include "ble_manager.h"

#include "ble/ble_connection.h"
#include "ble/ble_scanner.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <services/gap/ble_svc_gap.h>

static const char *TAG = "ble_manager";

/**
 * Callback that is called when the NimBLE stack is synchronized and ready.
 */
static void on_sync(void)
{
    start_scan();
}

static void ble_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run(); // This blocks until the stack is stopped
    nimble_port_freertos_deinit();
}

void ble_manager_task(void *pvParameter)
{
    // Initialize and start the NimBLE stack
    nimble_port_init();

    // Host configuration with our sync callback
    ble_hs_cfg.sync_cb = on_sync;

    // Configure GAP service for central mode
    ble_svc_gap_init();

    // Start the NimBLE host task
    nimble_port_freertos_init(ble_host_task); // Not a separate task, can run in the app task

    vTaskDelete(NULL);
}

bool ble_found_devices(void)
{
    return get_device_count() > 0;
}

void ble_connect_to_device(int index)
{
    device_info_t *device = get_device(index);
    if (device != NULL)
    {
        ble_connect(device);
    }
}
