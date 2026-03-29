idf_component_register(
    SRCS
        "src/iris.c"
        "src/iris_storage.c"
        "src/iris_coap.c"
        "src/iris_discovery.c"
        "src/iris_master.c"
        "src/iris_inventory.c"
    INCLUDE_DIRS "include"
    REQUIRES
        openthread
        spiffs
        freertos
        log
        json
        vfs
)
