idf_component_register(SRCS
        src/persistence_manager.c
        INCLUDE_DIRS "include"
        REQUIRES
        nvs_flash
)
