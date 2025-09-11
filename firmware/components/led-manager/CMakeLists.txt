idf_component_register(SRCS
            src/led_manager.cpp
            src/led_status.c
        INCLUDE_DIRS "include"
        PRIV_REQUIRES
            u8g2
            esp_event
            esp_timer
            persistence-manager
)
