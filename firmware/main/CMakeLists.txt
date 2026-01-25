idf_component_register(SRCS 
        src/main.cpp
        src/app_task.cpp
        src/button_handling.c
        src/i2c_checker.c
        src/hal/u8g2_esp32_hal.c
    INCLUDE_DIRS "include"
    PRIV_REQUIRES
        analytics
        insa
        connectivity-manager
        led-manager
        persistence-manager
        simulator
        u8g2
        hal
        nvs_flash
        esp_timer
        esp_event
        esp_wifi
        app_update
        rmaker_common
        driver
        my_mqtt_client
)

spiffs_create_partition_image(storage ../storage FLASH_IN_PROJECT)
