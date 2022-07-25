idf_component_register(SRCS "user_data.c" "user_gui.c" "lcd_display.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES board_define
                    PRIV_REQUIRES lvgl
                    PRIV_REQUIRES lvgl_esp32_drivers
                    PRIV_REQUIRES input_device
                    PRIV_REQUIRES weather
                    PRIV_REQUIRES app_smartconfig
                    )
