/********************************************************************************************************
 * @file     app_smartconfig.c
 *
 * @brief    app_smartconfig c file
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

/* Includes ------------------------------------------------------------------*/
#include "app_smartconfig.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
// #include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"

/* External variables --------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

static const char *TAG = "APP SC";

#define WIFI_NVS_NAMESPACE              "wifi_data"
#define WIFI_KEY_SSID                   "wifi_ssid"
#define WIFI_KEY_PWD                    "wifi_pwd"


/* Private typedef -----------------------------------------------------------*/
#define SSID_MAX_LEN                    32
#define PWD_MAX_LEN                     64
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
char default_ssid[] = "ssid";
char default_pwd[] = "password";
char *user_ssid, *user_pwd;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;

/* Private function prototypes -----------------------------------------------*/
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void smartconfig_example_task(void *parm);
static void wifi_nvs_data_init(void);
static void wifi_nvs_date_update(void);

/* User code -----------------------------------------------------------------*/
/**
 * @brief  app_sc_wifi_start
 * @note   start wifi using smartconfig.
 * @param  None.
 * @retval None.
 */
void app_sc_wifi_start(void)
{
    wifi_nvs_data_init();
    ESP_ERROR_CHECK(esp_netif_init());

    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config_t));         /* must clr it before use  */
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    memcpy(wifi_config.sta.ssid, user_ssid, strlen(user_ssid));
    memcpy(wifi_config.sta.password, user_pwd, strlen(user_pwd));\

    ESP_LOGI(TAG, "SSID :%s", wifi_config.sta.ssid);
    ESP_LOGI(TAG, "PWD :%s", wifi_config.sta.password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_ERROR_CHECK(esp_wifi_start());
}

/**
 * @brief  event_handler
 * @note   None.
 * @param  None.
 * @retval None.
 */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    static uint8_t retry = 0;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {   
        ESP_LOGI(TAG, "WiFi connect fail!");
        retry++;
        if(retry < 5)
        {
            esp_wifi_connect();
        }
        else
        {
            ESP_LOGI(TAG, "WiFi connect fail over 5 times!, start smartconfig..");
            xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
        }
        
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "WiFi connected!");
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        wifi_nvs_date_update();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG, "Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));        
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        
        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i = 0; i < 33; i++)
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        memset(user_ssid, 0, SSID_MAX_LEN);
        memset(user_pwd, 0, PWD_MAX_LEN);
        memcpy(user_ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(user_pwd, evt->password, sizeof(evt->password));
        
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
    else
    {
        printf("event base = %d. event id = %d.", event_base, event_id);
    }
}

/**
 * @brief  event_handler
 * @note   None.
 * @param  None.
 * @retval None.
 */
static void smartconfig_example_task(void *parm)
{
    EventBits_t uxBits;

    // ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS));
    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    // ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    esp_smartconfig_start(&cfg);
    while (1)
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if (uxBits & CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

/**
 * @brief  wifi_nvs_data_init
 * @note   从NVS中读取wifi配置参数.
 * @param  None.
 * @retval None.
 */
static void wifi_nvs_data_init(void)
{
    nvs_handle_t wifi_data_handler;
    esp_err_t err;
    size_t len = 0;

    user_ssid = malloc(sizeof(char)*SSID_MAX_LEN);
    user_pwd = malloc(sizeof(char)*PWD_MAX_LEN);
    // Open
    err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READONLY, &wifi_data_handler);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Open NVS namespace fail, Use default ssid and pwd; %s.", esp_err_to_name(err));
        memset(user_ssid, 0, SSID_MAX_LEN);
        memset(user_pwd, 0, PWD_MAX_LEN);
        memcpy(user_ssid, default_ssid, sizeof(default_ssid));
        memcpy(user_pwd, default_pwd, sizeof(default_pwd));
        ESP_LOGI(TAG, "SSID: %s ; PWD: %s.", user_ssid, user_pwd);

        nvs_close(wifi_data_handler);

        return ;
    }
    else
    {
        len = SSID_MAX_LEN;
        err = nvs_get_str(wifi_data_handler, WIFI_KEY_SSID, user_ssid, &len);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "Get ssid fail, Use default ssid and pwd; %s.", esp_err_to_name(err));
            memset(user_ssid, 0, SSID_MAX_LEN);
            memset(user_pwd, 0, PWD_MAX_LEN);
            memcpy(user_ssid, default_ssid, sizeof(default_ssid));
            memcpy(user_pwd, default_pwd, sizeof(default_pwd));
            ESP_LOGI(TAG, "SSID: %s ; PWD: %s.", user_ssid, user_pwd);

            nvs_close(wifi_data_handler);

            return ;
        }

        len = PWD_MAX_LEN;
        err = nvs_get_str(wifi_data_handler, WIFI_KEY_PWD, user_pwd, &len);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "Get ssid fail, Use default ssid and pwd; %s.", esp_err_to_name(err));
            memset(user_ssid, 0, SSID_MAX_LEN);
            memset(user_pwd, 0, PWD_MAX_LEN);
            memcpy(user_ssid, default_ssid, sizeof(default_ssid));
            memcpy(user_pwd, default_pwd, sizeof(default_pwd));
            ESP_LOGI(TAG, "SSID: %s ; PWD: %s.", user_ssid, user_pwd);
            nvs_close(wifi_data_handler);

            return ;
        }

        ESP_ERROR_CHECK( nvs_commit(wifi_data_handler) );     /* 提交NVS数据 */
        nvs_close(wifi_data_handler);                         /* 关闭NVS操作空间 */

        ESP_LOGI(TAG, "Get nvs ssid and pwd, %s : %s ", user_ssid, user_pwd);
    }
}

/**
 * @brief  wifi_nvs_date_update
 * @note   更新NVS中的wifi配置参数.
 * @param  None.
 * @retval None.
 */
static void wifi_nvs_date_update(void)
{
    nvs_handle_t wifi_data_handler;
    esp_err_t err;

    // Open
    err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &wifi_data_handler);
    if (err != ESP_OK)
    {
        ESP_ERROR_CHECK( nvs_commit(wifi_data_handler) );     /* 提交NVS数据 */
        nvs_close(wifi_data_handler);
        ESP_LOGW(TAG, "Open NVS namespace fail; %s.", esp_err_to_name(err));
        return ;
    }
    else
    {
        err = nvs_set_str(wifi_data_handler, WIFI_KEY_SSID, user_ssid);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "Write wifi ssid to nvs fail; %s.", esp_err_to_name(err));
            ESP_ERROR_CHECK( nvs_commit(wifi_data_handler) );     /* 提交NVS数据 */
            nvs_close(wifi_data_handler);

            return ;
        }

        err = nvs_set_str(wifi_data_handler, WIFI_KEY_PWD, user_pwd);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "Write wifi pwd to nvs fail; %s.", esp_err_to_name(err));
            ESP_ERROR_CHECK( nvs_commit(wifi_data_handler) );     /* 提交NVS数据 */
            nvs_close(wifi_data_handler);
            
            return ;
        }

        ESP_ERROR_CHECK( nvs_commit(wifi_data_handler) );     /* 提交NVS数据 */
        nvs_close(wifi_data_handler);

        memset(user_ssid, 0, SSID_MAX_LEN);
        memset(user_pwd, 0, PWD_MAX_LEN);

        free(user_ssid);
        free(user_pwd);
    }
}

/**
 * @brief  is_connect_to_ap
 * @note   判断设备是否连接至路由器.
 * @param  None.
 * @retval None.
 */
bool is_connect_to_ap(void)
{
    EventBits_t bits;

    bits = xEventGroupGetBits(s_wifi_event_group);

    if(bits & CONNECTED_BIT)
    {
        return true;
    }

    return false;
}
/*********************************END OF FILE**********************************/
