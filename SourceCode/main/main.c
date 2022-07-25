#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "driver/gpio.h"
#include "nvs_flash.h"

#include "input_device.h"
#include "lcd_display.h"
#include "ledc_fade.h"
#include "app_smartconfig.h"
#include "app_sntp.h"
#include "app_console.h"
#include "weather.h"
#include "user_data.h"


#define TAG         "MAIN"



void app_main(void)
{   
    ESP_ERROR_CHECK( nvs_flash_init() );
    
    lcd_display_init();
    ledc_fade_init();
    app_sc_wifi_start();
    app_console_init();
    app_sntp_init();
    weather_init();
    user_data_init();
}
