/********************************************************************************************************
* @file     app_console.c
* 
* @brief    app_console c file
* 
* @author   404Zen
* 
* @date     2022-07-22  
* 
* @version  Ver: 0.1
* 
* @attention 
* 
* None.
* 
*******************************************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "app_console.h"

#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_console.h"
// #include "esp_vfs_fat.h"             /* I don't want to support console history */
#include "cmd_system.h"
#include "argtable3/argtable3.h"
#include "weather.h"

/* Defines ------------------------------------------------------------------*/

/* Variables ----------------------------------------------------------------*/

/* Static functions prototypes ----------------------------------------------*/
static void register_weather_key_add(void);
static int set_weather_key_cmd(int argc, char **argv);
/* Functions declears -------------------------------------------------------*/
/**
 * @brief  app_console_init
 * @note   控制台程序初始化.
 * @param  None.
 * @retval None.
**/
void app_console_init(void)
{   
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();

    repl_config.prompt = "esp32c3>";

    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));

    register_system();

    printf("\n ==============================================================\n");
    printf("\r\n\
            \t _____ ____  ____ _________   ____ _____ \n\
            \t| ____/ ___||  _ \\___ /___ \\ / ___|___ / \n\
            \t|  _| \\___ \\| |_) ||_ \\ __) | |     |_ \\ \n\
            \t| |___ ___) |  __/___) / __/| |___ ___) | \n\
            \t|_____|____/|_|  |____/_____|\\____|____/ \n\n");
    printf(" ==============================================================\n\n");

    ESP_ERROR_CHECK(esp_console_start_repl(repl));

    /* Register user define cmd */
    register_weather_key_add();
}

typedef struct
{
    struct arg_str *location;
    struct arg_str *key;
    struct arg_end *end;
}w_data_args_t;

static w_data_args_t w_data_args;


/**
 * @brief  register_weather_key_add
 * @note   添加天气应用的key.
 * @param  None.
 * @retval None.
**/
static void register_weather_key_add(void)
{   
    w_data_args.location = arg_str1(NULL, NULL, "<location>", "location id of your city. you can search in: https://geoapi.qweather.com/v2/city/lookup?location=beijing&key=your_key");
    w_data_args.key = arg_str0(NULL, NULL, "<key>", "Key of weather applicatoin");
    w_data_args.end = arg_end(2);

    const esp_console_cmd_t weather_data = {
        .command = "weather_param",
        .help = "Set weather parameters of weather application",
        .hint = NULL,
        .func = &set_weather_key_cmd,
        .argtable = &w_data_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&weather_data));
}

static int set_weather_key_cmd(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&w_data_args);

    if(nerrors != 0)
    {
        arg_print_errors(stderr, w_data_args.end, argv[0]);
        return -1;
    }


    if(strlen(w_data_args.location->sval[0]) < 4)
    {
        printf("weather location id too short, fail.\r\n");
        return -2;
    }
    

    if(strlen(w_data_args.key->sval[0]) < 8)
    {
        printf("weather key is too short, fail.\r\n");
        return -2;
    }
    else
    {
        if(0 == set_weather_parameter(w_data_args.location->sval[0], w_data_args.key->sval[0]))
        // if(0 == set_weather_key(w_data_args.key->sval[0]))
        {
            printf("Weather parameter is set.\r\n");
        }
        else
        {
            printf("Weather parameter set fail.\r\n");
        }
    }

    
    return 0;
}
/******************************************* EOF *****************************************************/
