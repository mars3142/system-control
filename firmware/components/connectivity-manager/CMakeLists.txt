idf_component_register(SRCS
            src/ble/ble_connection.c
            src/ble/ble_scanner.c
            src/ble_manager.c
            src/dns_hijack.c
            src/wifi_manager.c
        INCLUDE_DIRS "include"
        REQUIRES
            bt
            driver
            nvs_flash
            esp_insights
            led-manager
            api-server
)
