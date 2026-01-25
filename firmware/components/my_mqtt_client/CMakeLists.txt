idf_component_register(
    SRCS "src/my_mqtt_client.c"
    INCLUDE_DIRS "include"
    REQUIRES 
        mqtt
        app_update
)