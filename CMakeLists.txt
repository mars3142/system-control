cmake_minimum_required(VERSION 3.30)

if (DEFINED ENV{IDF_PATH})
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(system_control)
    return()
else ()
    set(MAJOR_VERSION 0)
    set(MINOR_VERSION 0)
    set(MICRO_VERSION 1)

    project(
            SystemControl
            LANGUAGES CXX C
            VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}"
    )

    set(CMAKE_C_STANDARD 23)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED YES)
    set(CMAKE_CXX_EXTENSIONS YES)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_INCLUDE_PATH include)
    set(CMAKE_SOURCE_DIR src)
    set(CMAKE_MODULE_PATH cmake)

    include(FetchContent)
    function(include_dependency libName gitURL gitTag)
        FetchContent_Declare(${libName}
                GIT_REPOSITORY ${gitURL}
                GIT_TAG ${gitTag}
                GIT_SHALLOW TRUE
                GIT_PROGRESS FALSE
        )
        FetchContent_MakeAvailable(${libName})
    endfunction()

    include_dependency(SDL3 https://github.com/libsdl-org/SDL release-3.2.x)
    include_dependency(SDL_image https://github.com/libsdl-org/SDL_image release-3.2.x)
    include_dependency(SDL_ttf https://github.com/libsdl-org/SDL_ttf release-3.2.x)
    include_dependency(u8g2 https://github.com/olikraus/u8g2 master)

    add_subdirectory(components)

    configure_file(
            "${CMAKE_SOURCE_DIR}/Version.h.in"
            "${CMAKE_SOURCE_DIR}/Version.h"
    )

    include_directories(
            ${CMAKE_INCLUDE_PATH}
            ${CMAKE_SOURCE_DIR}
            ${PROJECT_BINARY_DIR}/src
    )

    add_executable(${PROJECT_NAME}
            WIN32 MACOSX_BUNDLE
            ${CMAKE_SOURCE_DIR}/main.cpp
            ${CMAKE_SOURCE_DIR}/Common.cpp
            ${CMAKE_SOURCE_DIR}/debug/DebugOverlay.cpp
            ${CMAKE_SOURCE_DIR}/hal/u8x8_hal_sdl.cpp
            ${CMAKE_SOURCE_DIR}/manager/PersistenceManager.cpp
            ${CMAKE_SOURCE_DIR}/manager/ResourceManager.cpp
            ${CMAKE_SOURCE_DIR}/model/AppContext.cpp
            ${CMAKE_SOURCE_DIR}/model/Window.cpp
            ${CMAKE_SOURCE_DIR}/ui/Device.cpp
            ${CMAKE_SOURCE_DIR}/ui/Matrix.cpp
            ${CMAKE_SOURCE_DIR}/ui/UIWidget.cpp
            ${CMAKE_SOURCE_DIR}/ui/widgets/Button.cpp
            ${CMAKE_SOURCE_DIR}/ui/widgets/D_Pad.cpp
    )

    if (APPLE)
        # copy assets to bundle directory
        set(ASSETS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets)
        set(ASSETS_DEST_DIR $<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources)

        if (EXISTS ${ASSETS_SRC_DIR})
            add_custom_command(
                    TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SRC_DIR} ${ASSETS_DEST_DIR}
                    COMMENT "Copying assets to bundle via custom command"
                    VERBATIM
            )
        else ()
            message(WARNING "Assets source directory ${ASSETS_SRC_DIR} does not exist. Skipping custom command.")
        endif ()
    elseif (WINDOWS)
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        )
    endif ()
    target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_USE_CALLBACKS)

    target_link_libraries(${PROJECT_NAME} PRIVATE
            ImGui
            insa
            SDL3::SDL3
            SDL3_image::SDL3_image
            SDL3_ttf::SDL3_ttf
            u8g2
    )
endif ()
