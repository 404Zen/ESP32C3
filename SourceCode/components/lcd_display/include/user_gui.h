/********************************************************************************************************
* @file     user_gui.h
* 
* @brief    user_gui header file
* 
* @author   wujianping@nbdeli.com
* 
* @date     2022-07-06  
* 
* @version  Ver: 0.1
* 
* @attention 
* 
* None.
* 
*******************************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_GUI_H__
#define __USER_GUI_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/

/* Exported defines ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void user_gui_task(void);
void user_startup_page(void);

/*  */
void wl_set_ssid_label(char *ssid);
void wl_set_ip_label(char *ip);


#endif /* __USER_GUI_H__ */
/*********************************END OF FILE**********************************/
