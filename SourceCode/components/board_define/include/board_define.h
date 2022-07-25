/********************************************************************************************************
* @file     board_define.c
* 
* @brief    board_define header file
* 
* @author   wujianping@nbdeli.com
* 
* @date     2022-06-29  
* 
* @version  Ver: 0.1
* 
* @attention 
* 
* None.
* 
*******************************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BOARD_DEFINE_H__
#define __BOARD_DEFINE_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

/* Exported defines ----------------------------------------------------------*/

/* IO definition For  Luat ESP32C3 Core and Expansion LCD 0.96 Inch */
#define LED_D4                          12
// #define LED_D5                          13                  /* 与摇杆冲突 */

// #define BTN_BOOT                        9
#define R_KEY                           9                   /* Same as BTN_BOOT */
#define L_KEY                           5                   
#define UP_KEY                          8
#define DN_KEY                          13                  /* Same as LED D5 */
#define CENTER_KEY                      4

/* LCD ST7735S. 80*160 pixels */
#define LCD_BL                          11
#define LCD_CS                          7
#define LCD_DC                          6
#define LCD_RES                         10
#define LCD_SDA                         3
#define LCD_SCL                         2


/* NTP server config */
#define NTP_SERVER_0                    "cn.ntp.org.cn"
#define NTP_SERVER_1                    "pool.ntp.org"
#define NTP_SERVER_2                    "ntp.aliyun.com"

/* Weather parameter config */
#define WEATHER_REFRESH_TIME            300                 /* 单位为秒， 建议每300秒刷新一次即可 */
#define WEATHER_SERVER                  "devapi.qweather.com"
#define WEATHER_SERVER_PORT             "443"
#define WEATHER_SERVER_URL              "https://devapi.qweather.com/v7/weather/now?lang=en"

#endif /* __BOARD_DEFINE_H__ */
/*********************************END OF FILE**********************************/



