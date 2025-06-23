if (DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS
            src/hal_esp32/PersistenceManager.cpp
            INCLUDE_DIRS "include"
            REQUIRES
                nvs_flash
    )
    return()
endif ()

cmake_minimum_required(VERSION 3.30)
project(persistence-manager)

add_library(${PROJECT_NAME} STATIC
        src/hal_native/PersistenceManager.cpp
)

include_directories(include)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3::SDL3
)
