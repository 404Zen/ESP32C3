idf_component_register(SRCS "app_smartconfig.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES nvs_flash
                    PRIV_REQUIRES lcd_display)
