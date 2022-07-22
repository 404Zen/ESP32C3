/********************************************************************************************************
* @file     weather.h
* 
* @brief    weather header file
* 
* @author   wujianping@nbdeli.com
* 
* @date     2022-07-11  
* 
* @version  Ver: 0.1
* 
* @attention 
* 
* None.
* 
*******************************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WEATHER_H__
#define __WEATHER_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/

/* Exported defines ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    char temp[4];
    char text[16];
}weather_data_t;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void weather_init(void);
// int get_weather_data(char *data, size_t *data_len);
void response_data_process(char *data, size_t len);
void get_weather_data(weather_data_t *data);


#endif /* __WEATHER_H__ */
/*********************************END OF FILE**********************************/
