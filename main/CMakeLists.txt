idf_component_register(SRCS 
        main.cpp
        app_task.cpp
        PersistenceManager.cpp
        button_handling.c
        hal/u8g2_esp32_hal.c
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        insa
        led-manager
        u8g2
        driver
        esp_timer
        esp_event
        nvs_flash
)
