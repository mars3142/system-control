idf_component_register(SRCS 
                        "src/simulator.c"
                        "src/storage.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES
                        led-manager
                        spiffs
)
