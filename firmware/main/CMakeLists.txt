idf_component_register(SRCS 
        main.cpp
        app_task.cpp
        button_handling.c
        i2c_checker.c
        hal/u8g2_esp32_hal.c
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        analytics
        insa
        connectivity-manager
        led-manager
        persistence-manager
        simulator
        u8g2
        nvs_flash
        esp_timer
        esp_event
        esp_wifi
        app_update
        rmaker_common
)

spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
