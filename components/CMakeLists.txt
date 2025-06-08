if (DEFINED ENV{IDF_PATH})
    return()
endif ()

add_library(components INTERFACE)

add_subdirectory(imgui)
add_subdirectory(justus)
add_subdirectory(peter)

target_link_libraries(components INTERFACE ImGui)
