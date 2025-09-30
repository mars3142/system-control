idf_component_register(SRCS
            src/color.c
            src/led_status.c
            src/led_strip_ws2812.c
        INCLUDE_DIRS "include"
        PRIV_REQUIRES
            insa
            u8g2
            esp_event
            esp_timer
            persistence-manager
            simulator
)
