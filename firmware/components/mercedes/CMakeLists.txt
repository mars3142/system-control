idf_component_register(SRCS
            src/DynamicMenuBuilder.cpp
        INCLUDE_DIRS "include"
        PRIV_REQUIRES
            heimdall
            json
            insa
)
