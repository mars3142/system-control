idf_component_register(SRCS 
        main.cpp
        app_task.cpp
        button_handling.c
        hal/u8g2_esp32_hal.c
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        insa
        ble-manager
        led-manager
        persistence-manager
        u8g2
        nvs_flash
        driver
        esp_timer
        esp_event
)
