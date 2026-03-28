idf_component_register(SRCS
            src/skuld.c
        INCLUDE_DIRS "include"
        PRIV_REQUIRES
            lwip
            esp_http_client
            nvs_flash
            cjson
)
