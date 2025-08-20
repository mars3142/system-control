if (DEFINED ENV{IDF_PATH})
    return()
endif ()

cmake_minimum_required(VERSION 3.30)
project(ImGui)

add_library(${PROJECT_NAME} STATIC
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_widgets.cpp
        imgui_tables.cpp

        imgui_impl_sdl3.cpp
        imgui_impl_sdlrenderer3.cpp
)

include_directories(include)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
