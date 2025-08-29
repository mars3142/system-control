if (NOT DEFINED ENV{IDF_PATH})
    add_library(components INTERFACE)

    add_subdirectory(imgui)
    add_subdirectory(insa)
    add_subdirectory(led-manager)
    add_subdirectory(persistence-manager)

    target_link_libraries(components INTERFACE ImGui)
endif ()