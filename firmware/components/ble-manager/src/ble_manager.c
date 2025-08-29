#include "ble_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"

static const char *TAG = "ble_manager";

// List of allowed manufacturer IDs
static const uint16_t ALLOWED_MANUFACTURERS[] = {
    0xC0DE, // mars3142
};
static const size_t NUM_MANUFACTURERS = sizeof(ALLOWED_MANUFACTURERS) / sizeof(uint16_t);

// Structure to cache device data
typedef struct
{
    ble_addr_t addr;
    uint16_t manufacturer_id;
    uint8_t manufacturer_data[31]; // Max. length of manufacturer data
    uint8_t manufacturer_data_len;
    char name[32];
    uint16_t service_uuids_16[10]; // Up to 10 16-bit Service UUIDs
    uint8_t service_uuids_16_count;
    ble_uuid128_t service_uuids_128[5]; // Up to 5 128-bit Service UUIDs
    uint8_t service_uuids_128_count;
    bool has_manufacturer;
    bool has_name;
    int8_t rssi;
} device_info_t;

static bool is_manufacturer_allowed(uint16_t company_id)
{
    for (size_t i = 0; i < NUM_MANUFACTURERS; i++)
    {
        if (ALLOWED_MANUFACTURERS[i] == company_id)
        {
            return true;
        }
    }
    return false;
}

static int ble_central_gap_event(struct ble_gap_event *event, void *arg);

/**
 * Starts the BLE scan process.
 */
static void start_scan(void)
{
    struct ble_gap_disc_params disc_params = {
        .filter_policy = 0,
        .limited = 0,
        .passive = 0,
        .filter_duplicates = 1,
    };

    int rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &disc_params, ble_central_gap_event, NULL);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Error starting scan; rc=%d", rc);
    }
}

#define MAX_DEVICES 10
static device_info_t devices[MAX_DEVICES];
static int device_count = 0;

// Helper function to find or create a device entry
static device_info_t *find_or_create_device(const ble_addr_t *addr)
{
    // Search for existing device
    for (int i = 0; i < device_count; i++)
    {
        if (memcmp(&devices[i].addr, addr, sizeof(ble_addr_t)) == 0)
        {
            return &devices[i];
        }
    }

    // Add new device
    if (device_count < MAX_DEVICES)
    {
        memset(&devices[device_count], 0, sizeof(device_info_t));
        memcpy(&devices[device_count].addr, addr, sizeof(ble_addr_t));
        devices[device_count].has_manufacturer = false;
        devices[device_count].has_name = false;
        strcpy(devices[device_count].name, "Unknown");
        return &devices[device_count++];
    }

    return NULL;
}

static int ble_central_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_disc_desc *disc;
    struct ble_hs_adv_fields fields;

    switch (event->type)
    {
    case BLE_GAP_EVENT_DISC: {
        disc = &event->disc;

        // Find or create device
        device_info_t *device = find_or_create_device(&disc->addr);
        if (device == NULL)
        {
            return 0;
        }

        // Update RSSI
        device->rssi = disc->rssi;

        // Parse advertising data
        memset(&fields, 0, sizeof(fields));
        ble_hs_adv_parse_fields(&fields, disc->data, disc->length_data);

        // Process manufacturer data
        if (fields.mfg_data != NULL && fields.mfg_data_len >= 2)
        {
            uint16_t company_id = fields.mfg_data[0] | (fields.mfg_data[1] << 8);
            device->manufacturer_id = company_id;
            device->has_manufacturer = true;

            // Store complete manufacturer data (incl. Company ID)
            device->manufacturer_data_len = fields.mfg_data_len;
            memcpy(device->manufacturer_data, fields.mfg_data, fields.mfg_data_len);
        }

        // Process name
        if (fields.name != NULL && fields.name_len > 0)
        {
            size_t copy_len = fields.name_len < sizeof(device->name) - 1 ? fields.name_len : sizeof(device->name) - 1;
            memcpy(device->name, fields.name, copy_len);
            device->name[copy_len] = '\0';
            device->has_name = true;
        }

        // Process 16-bit Service UUIDs
        if (fields.uuids16 != NULL && fields.num_uuids16 > 0)
        {
            for (int i = 0; i < fields.num_uuids16 && device->service_uuids_16_count < 10; i++)
            {
                // Check if UUID already exists
                bool exists = false;
                for (int j = 0; j < device->service_uuids_16_count; j++)
                {
                    if (device->service_uuids_16[j] == fields.uuids16[i].value)
                    {
                        exists = true;
                        break;
                    }
                }
                if (!exists)
                {
                    device->service_uuids_16[device->service_uuids_16_count++] = fields.uuids16[i].value;
                }
            }
        }

        // Process 128-bit Service UUIDs
        if (fields.uuids128 != NULL && fields.num_uuids128 > 0)
        {
            for (int i = 0; i < fields.num_uuids128 && device->service_uuids_128_count < 5; i++)
            {
                // Check if UUID already exists
                bool exists = false;
                for (int j = 0; j < device->service_uuids_128_count; j++)
                {
                    if (memcmp(&device->service_uuids_128[j], &fields.uuids128[i], sizeof(ble_uuid128_t)) == 0)
                    {
                        exists = true;
                        break;
                    }
                }
                if (!exists)
                {
                    memcpy(&device->service_uuids_128[device->service_uuids_128_count++], &fields.uuids128[i],
                           sizeof(ble_uuid128_t));
                }
            }
        }

        // Check if we have all data and the device is allowed
        if (device->has_manufacturer && is_manufacturer_allowed(device->manufacturer_id))
        {
            ESP_LOGI(TAG, "*** Allowed device found ***");
            ESP_LOGI(TAG, "  Name: %s", device->name);
            ESP_LOGI(TAG, "  Address: %02X:%02X:%02X:%02X:%02X:%02X", device->addr.val[5], device->addr.val[4],
                     device->addr.val[3], device->addr.val[2], device->addr.val[1], device->addr.val[0]);
            ESP_LOGI(TAG, "  Manufacturer ID: 0x%04X", device->manufacturer_id);
            ESP_LOGI(TAG, "  RSSI: %d dBm", device->rssi);

            // Print Service UUIDs
            if (device->service_uuids_16_count > 0)
            {
                ESP_LOGI(TAG, "  16-bit Service UUIDs (%d):", device->service_uuids_16_count);
                for (int i = 0; i < device->service_uuids_16_count; i++)
                {
                    const char *name = "";
                    // Known Service UUIDs
                    switch (device->service_uuids_16[i])
                    {
                    case 0x180A:
                        name = " (Device Information)";
                        break;
                    case 0x180F:
                        name = " (Battery Service)";
                        break;
                    case 0x1801:
                        name = " (Generic Attribute)";
                        break;
                    case 0x1800:
                        name = " (Generic Access)";
                        break;
                    case 0x181A:
                        name = " (Environmental Sensing)";
                        break;
                    default:
                        if (device->service_uuids_16[i] >= 0xA000)
                        {
                            name = " (Custom)";
                        }
                        break;
                    }
                    ESP_LOGI(TAG, "    - 0x%04X%s", device->service_uuids_16[i], name);
                }
            }

            if (device->service_uuids_128_count > 0)
            {
                ESP_LOGI(TAG, "  128-bit Service UUIDs (%d):", device->service_uuids_128_count);
                for (int i = 0; i < device->service_uuids_128_count; i++)
                {
                    char uuid_str[37]; // UUID string format
                    snprintf(uuid_str, sizeof(uuid_str),
                             "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                             device->service_uuids_128[i].value[15], device->service_uuids_128[i].value[14],
                             device->service_uuids_128[i].value[13], device->service_uuids_128[i].value[12],
                             device->service_uuids_128[i].value[11], device->service_uuids_128[i].value[10],
                             device->service_uuids_128[i].value[9], device->service_uuids_128[i].value[8],
                             device->service_uuids_128[i].value[7], device->service_uuids_128[i].value[6],
                             device->service_uuids_128[i].value[5], device->service_uuids_128[i].value[4],
                             device->service_uuids_128[i].value[3], device->service_uuids_128[i].value[2],
                             device->service_uuids_128[i].value[1], device->service_uuids_128[i].value[0]);
                    ESP_LOGI(TAG, "    - %s", uuid_str);
                }
            }

            // Print manufacturer data (without Company ID, i.e., from byte 2)
            if (device->manufacturer_data_len > 2)
            {
                int payload_len = device->manufacturer_data_len - 2;
                ESP_LOGI(TAG, "  Manufacturer Data (%d bytes):", payload_len);
                ESP_LOG_BUFFER_HEX_LEVEL(TAG, &device->manufacturer_data[2], payload_len, ESP_LOG_INFO);

                // Print data byte by byte for better readability
                ESP_LOGI(TAG, "  Data interpretation:");
                for (int i = 0; i < payload_len; i++)
                {
                    ESP_LOGI(TAG, "    - Byte %d: 0x%02X (%d)", i, device->manufacturer_data[i + 2],
                             device->manufacturer_data[i + 2]);
                }

                // Optional: Interpret data as 16-bit values (if the number of bytes is even)
                if (payload_len >= 2 && (payload_len % 2 == 0))
                {
                    ESP_LOGI(TAG, "  As 16-bit values:");
                    for (int i = 0; i < payload_len; i += 2)
                    {
                        uint16_t value = device->manufacturer_data[i + 2] | (device->manufacturer_data[i + 3] << 8);
                        ESP_LOGI(TAG, "    - Word %d: 0x%04X (%d)", i / 2, value, value);
                    }
                }
            }
            else
            {
                ESP_LOGI(TAG, "  No manufacturer payload data");
            }
        }

        return 0;
    }

    case BLE_GAP_EVENT_DISC_COMPLETE: {
        ESP_LOGI(TAG, "Discovery complete, restarting scan...");
        // Optional: Reset device list
        device_count = 0;
        start_scan();
        return 0;
    }

    default:
        break;
    }
    return 0;
}

/**
 * Callback that is called when the NimBLE stack is synchronized and ready.
 */
static void on_sync(void)
{
    // The stack is ready, we can start the scan.
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
