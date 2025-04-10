if (DEFINED ENV{IDF_PATH})
idf_component_register(SRCS 
        src/common/PSMenu.cpp
        src/common/ScrollBar.cpp
        src/common/Widget.cpp
        src/data/MenuItem.cpp
        src/ui/LightMenu.cpp
        src/ui/MainMenu.cpp
        src/ui/SettingsMenu.cpp
        src/ui/SplashScreen.cpp
        INCLUDE_DIRS "include"
)
return()
endif()

cmake_minimum_required(VERSION 3.30)
project(justus)

add_library(${PROJECT_NAME} STATIC
        src/common/PSMenu.cpp
        src/common/ScrollBar.cpp
        src/common/Widget.cpp
        src/data/MenuItem.cpp
        src/ui/LightMenu.cpp
        src/ui/MainMenu.cpp
        src/ui/SettingsMenu.cpp
        src/ui/SplashScreen.cpp
)

include_directories(include)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} PRIVATE
        u8g2
        peter
)
