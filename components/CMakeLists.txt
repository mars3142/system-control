if (DEFINED ENV{IDF_PATH})
    return()
endif ()

add_library(components INTERFACE)

add_subdirectory(imgui)
add_subdirectory(insa)
add_subdirectory(ruth)

target_link_libraries(components INTERFACE ImGui)
