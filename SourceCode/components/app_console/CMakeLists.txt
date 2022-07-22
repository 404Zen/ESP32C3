idf_component_register(SRCS "app_console.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES console
                    PRIV_REQUIRES cmd_system
                    PRIV_REQUIRES weather)
