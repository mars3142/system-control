if (DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS
                persistence.c
            INCLUDE_DIRS "include"
            PRIV_REQUIRES
                nvs_flash
    )
    return()
endif ()

cmake_minimum_required(VERSION 3.30)
project(ruth)

add_library(${PROJECT_NAME} STATIC
        persistence.c
)

include_directories(include)

target_include_directories(${PROJECT_NAME} PUBLIC include)
