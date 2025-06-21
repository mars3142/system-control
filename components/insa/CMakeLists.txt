# Definiere die Quelldateien in einer Variable
set(SOURCE_FILES
    src/common/InactivityTracker.cpp
    src/common/Menu.cpp
    src/common/ScrollBar.cpp
    src/common/Widget.cpp
    src/data/MenuItem.cpp
    src/ui/LightMenu.cpp
    src/ui/LightSettingsMenu.cpp
    src/ui/MainMenu.cpp
    src/ui/ScreenSaver.cpp
    src/ui/SettingsMenu.cpp
    src/ui/SplashScreen.cpp
)

if (DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS
            ${SOURCE_FILES}
            INCLUDE_DIRS "include"
            PRIV_REQUIRES
                u8g2
                led-manager
    )
    return()
endif ()

cmake_minimum_required(VERSION 3.30)
project(insa)

add_library(${PROJECT_NAME} STATIC
        ${SOURCE_FILES}
)

include_directories(include)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PRIVATE
        u8g2
        led-manager
)