idf_component_register(SRCS
            src/screensaver/clock_screensaver.cpp
            src/hermes.cpp
        INCLUDE_DIRS "include"
        REQUIRES
            mercedes
            simulator
            u8g2
)
