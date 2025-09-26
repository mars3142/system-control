idf_component_register(SRCS 
                        "simulator.c"
                        "storage.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES
                        led-manager
                        spiffs
)
