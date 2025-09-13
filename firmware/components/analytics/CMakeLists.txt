idf_component_register(SRCS
            src/analytics.c
        INCLUDE_DIRS "include"
        REQUIRES
            rmaker_common
            esp_insights
            rmaker_common
)

target_add_binary_data(${COMPONENT_TARGET} "insights_auth_key.txt" TEXT)
