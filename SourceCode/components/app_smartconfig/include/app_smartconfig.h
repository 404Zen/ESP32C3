/********************************************************************************************************
* @file     app_smartconfig.h
* 
* @brief    app_smartconfig header file
* 
* @author   wujianping@nbdeli.com
* 
* @date     2022-07-04  
* 
* @version  Ver: 0.1
* 
* @attention 
* 
* None.
* 
*******************************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_SMARTCONFIG_H__
#define __APP_SMARTCONFIG_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_wifi.h"

/* Exported functions prototypes ---------------------------------------------*/
void app_sc_wifi_start(void);
bool is_connect_to_ap(void);
void get_ap_info(wifi_ap_record_t *ap_info);
void get_ip_info(esp_netif_ip_info_t *ip_info);




#endif /* __APP_SMARTCONFIG_H__ */
/*********************************END OF FILE**********************************/
