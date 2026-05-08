idf_component_register(SRCS
            src/dns_hijack.c
            src/thread_manager.c
            src/wifi_manager.c
        INCLUDE_DIRS "include"
        REQUIRES
            driver
            nvs_flash
            openthread
            vfs
            skuld
            led-manager
            bifrost
)
