if (DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS
                            bob.c
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES
                            bt
                            ruth
    )
    return()
endif ()