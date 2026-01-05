idf_component_register(SRCS
            src/api_server.c
            src/api_handlers.c
            src/websocket_handler.c
        INCLUDE_DIRS "include"
        REQUIRES
            esp_http_server
            mdns
            esp_wifi
            esp_netif
            esp_event
            json
            simulator
)
