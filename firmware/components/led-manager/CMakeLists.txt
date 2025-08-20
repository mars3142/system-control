if (DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS
                src/hal_esp32/led_manager.cpp
            INCLUDE_DIRS "include"
            PRIV_REQUIRES
                u8g2
                esp_event
                persistence-manager
    )
    return()
endif ()

cmake_minimum_required(VERSION 3.30)
project(led-manager)

add_library(${PROJECT_NAME} STATIC
        src/hal_native/led_manager.cpp
        src/hal_native/Matrix.cpp
)

include_directories(include)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3::SDL3
        persistence-manager
)
