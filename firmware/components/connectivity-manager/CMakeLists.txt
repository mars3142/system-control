idf_component_register(SRCS
            src/ble_manager.c
            src/wifi_manager.c
        INCLUDE_DIRS "include"
        PRIV_REQUIRES
            bt
            driver
            nvs_flash
            esp_insights
            led-manager
)
