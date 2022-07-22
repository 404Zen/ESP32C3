idf_component_register(SRCS "app_sntp.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES nvs_flash
                    PRIV_REQUIRES app_smartconfig
                    PRIV_REQUIRES board_define)
