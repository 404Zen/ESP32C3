/********************************************************************************************************
* @file     user_data.c
* 
* @brief    user_data c file
* 
* @author   404Zen
* 
* @date     2022-07-25  
* 
* @version  Ver: 0.1
* 
* @attention 
* 
* None.
* 
*******************************************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"

#include "app_smartconfig.h"
#include "user_gui.h"
#include "user_data.h"


/* Defines ------------------------------------------------------------------*/
#define TAG         "USER_DATA"
/* Variables ----------------------------------------------------------------*/

/* Static functions prototypes ----------------------------------------------*/
static void user_data_sync_task(void *arg);
/* Functions declears -------------------------------------------------------*/
/**
 * @brief  user_data_init
 * @note   用户数据接口初始化.
 * @param  None.
 * @retval None.
**/
void user_data_init(void)
{
    xTaskCreate(user_data_sync_task, "user_data_sync_task", 4096, NULL, 5, NULL);
}

/**
 * @brief  user_data_sync_task
 * @note   用户数据同步.
 * @param  None.
 * @retval None.
**/
static void user_data_sync_task(void *arg)
{
    while (is_connect_to_ap() == false)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    /* 只需要同步一次的数据 */
    wifi_ap_record_t ap_info;
    esp_netif_ip_info_t ip_info;
    char ip_str[16];
    get_ap_info(&ap_info);
    get_ip_info(&ip_info);

    ESP_LOGI(TAG, "SSID: %s ", ap_info.ssid);
    ESP_LOGI(TAG, "CH: %d ", ap_info.primary);
    ESP_LOGI(TAG, "RSSI: %d ", ap_info.rssi);

    ESP_LOGI(TAG, "My IP: " IPSTR, IP2STR(&ip_info.ip));
    sprintf(ip_str, IPSTR, IP2STR(&ip_info.ip));
    ESP_LOGI(TAG, "My GW: " IPSTR, IP2STR(&ip_info.gw));
    ESP_LOGI(TAG, "My NETMASK: " IPSTR, IP2STR(&ip_info.netmask));
    

    wl_set_ssid_label((char *)ap_info.ssid);
    wl_set_ip_label(ip_str);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(50000));
    }
}


/******************************************* EOF *****************************************************/
