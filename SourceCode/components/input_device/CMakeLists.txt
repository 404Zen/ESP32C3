idf_component_register(SRCS "input_device.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES board_define
                    PRIV_REQUIRES button
                    PRIV_REQUIRES lw_oopc
                    )
