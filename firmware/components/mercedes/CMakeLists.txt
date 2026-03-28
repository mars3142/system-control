idf_component_register(SRCS
            src/mercedes.cpp
        INCLUDE_DIRS "include"
        REQUIRES
            heimdall
    PRIV_REQUIRES
            json
            message-manager
            persistence-manager
)
