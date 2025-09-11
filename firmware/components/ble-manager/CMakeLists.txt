idf_component_register(SRCS
            src/ble_manager.c
        INCLUDE_DIRS "include"
        PRIV_REQUIRES
            bt
            nvs_flash
)