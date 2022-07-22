idf_component_register(SRCS "app_blufi.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES nvs_flash
                    PRIV_REQUIRES bt 
                    )
