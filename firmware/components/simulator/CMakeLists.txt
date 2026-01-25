idf_component_register(SRCS 
                        "src/simulator.cpp"
                        "src/storage.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES
                        led-manager
                        persistence-manager
                        message-manager
                        spiffs
)
