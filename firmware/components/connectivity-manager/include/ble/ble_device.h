#pragma once

#include <host/ble_hs.h>
#include <host/util/util.h>
#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <stdint.h>

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
