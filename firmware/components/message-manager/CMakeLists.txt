idf_component_register(
    SRCS "src/message_manager.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES 
        persistence-manager
        my_mqtt_client
        app_update
)
