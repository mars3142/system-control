idf_component_register(SRCS 
    "main.cpp"
    "app_task.cpp"
    "button_handling.c"
    "hal/u8g2_esp32_hal.c"
    INCLUDE_DIRS "."
    REQUIRES
        insa
        ruth
        u8g2
        driver
        esp_timer
)
