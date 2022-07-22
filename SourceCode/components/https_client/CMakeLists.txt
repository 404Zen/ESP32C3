idf_component_register(SRCS "https_client.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES app_smartconfig
                    PRIV_REQUIRES nvs_flash
                    PRIV_REQUIRES mbedtls
                    PRIV_REQUIRES zlib
                    PRIV_REQUIRES weather
                    )
