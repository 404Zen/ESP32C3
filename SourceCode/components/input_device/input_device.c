/********************************************************************************************************
* @file     input_device.c
* 
* @brief    input_device c file
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

/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "input_device.h"
#include "board_define.h"
#include "lw_oopc.h"

#include "esp_log.h"
#include "esp_system.h"

/* iot solution compoents */
#include "iot_button.h"

/* External variables --------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define TAG         "Input Device"
/* Private typedef -----------------------------------------------------------*/
CLASS(Button)
{
    button_handle_t     button_handle;
    void                (*init)(void*, int32_t, int8_t);
    esp_err_t           (*button_callback_register)(button_handle_t, button_event_t, button_cb_t);
};

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
QueueHandle_t key_queue = NULL;

/* Private function prototypes -----------------------------------------------*/
static void gpio_key_init(void *t, int32_t gpio_num, int8_t active_level);

static void right_key_single_cb(void *args);
static void right_key_long_start_cb(void *args);
static void left_key_single_cb(void *args);
static void left_key_long_start_cb(void *args);
static void up_single_cb(void *args);
static void up_long_start_cb(void *args);
static void down_single_cb(void *args);
static void down_long_start_cb(void *args);
static void center_single_cb(void *args);
static void center_long_start_cb(void *args);

static void send_kvalue_to_queue_tail(uint32_t key_value);

CTOR(Button)
    FUNCTION_SETTING(init, gpio_key_init)
    FUNCTION_SETTING(button_callback_register, iot_button_register_cb)
END_CTOR
/* User code -----------------------------------------------------------------*/
 /**
  * @brief  funtion_brief
  * @note   None.
  * @param  None.
  * @retval None.
  */
void input_btn_init(void)
{
    Button *right_key;
    right_key = (Button*)ButtonNew();
    right_key->init(right_key, R_KEY, 0);
    right_key->button_callback_register(right_key->button_handle, BUTTON_SINGLE_CLICK, right_key_single_cb);
    right_key->button_callback_register(right_key->button_handle, BUTTON_LONG_PRESS_START, right_key_long_start_cb);

    Button *left_key;
    left_key = (Button*)ButtonNew();
    left_key->init(left_key, L_KEY, 0);
    left_key->button_callback_register(left_key->button_handle, BUTTON_SINGLE_CLICK, left_key_single_cb);
    left_key->button_callback_register(left_key->button_handle, BUTTON_LONG_PRESS_START, left_key_long_start_cb);

    Button *up_key;
    up_key = (Button*)ButtonNew();
    up_key->init(up_key, UP_KEY, 0);
    up_key->button_callback_register(up_key->button_handle, BUTTON_SINGLE_CLICK, up_single_cb);
    up_key->button_callback_register(up_key->button_handle, BUTTON_LONG_PRESS_START, up_long_start_cb);

    Button *down_key;
    down_key = (Button*)ButtonNew();
    down_key->init(down_key, DN_KEY, 0);
    down_key->button_callback_register(down_key->button_handle, BUTTON_SINGLE_CLICK, down_single_cb);
    down_key->button_callback_register(down_key->button_handle, BUTTON_LONG_PRESS_START, down_long_start_cb);

    Button *center_key;
    center_key = (Button*)ButtonNew();
    center_key->init(center_key, CENTER_KEY, 0);
    center_key->button_callback_register(center_key->button_handle, BUTTON_SINGLE_CLICK, center_single_cb);
    center_key->button_callback_register(center_key->button_handle, BUTTON_LONG_PRESS_START, center_long_start_cb);

    // uint32_t free_heap_size = esp_get_free_heap_size();
    // uint32_t free_iheap_size = esp_get_free_internal_heap_size();
    // uint32_t free_min_heap_size = esp_get_minimum_free_heap_size();

    // ESP_LOGI(TAG, "\n free_hap_size = %d. bytes. \n free_iheap_size = %d. \n free_min_heap_size = %d.", 
    //                 free_heap_size, free_iheap_size, free_min_heap_size);

    /* Free keys seems make some crash.. */
    // free(right_key);
    // free(left_key);
    // free(up_key);
    // free(down_key);
    // free(center_key);

    //  free_heap_size = esp_get_free_heap_size();
    //  free_iheap_size = esp_get_free_internal_heap_size();
    //  free_min_heap_size = esp_get_minimum_free_heap_size();

    // ESP_LOGI(TAG, "\n free_hap_size = %d. bytes. \n free_iheap_size = %d. \n free_min_heap_size = %d.", 
    //                 free_heap_size, free_iheap_size, free_min_heap_size);

    /* 创建一个按键队列 */
    key_queue = xQueueCreate(3, 4);
    
    if(key_queue == NULL)
    {
        ESP_LOGW(TAG, "key queue create fail");
    }
}

static void send_kvalue_to_queue_tail(uint32_t key_value)
{
    uint32_t kv = key_value;
    if(pdTRUE != xQueueSendToBack(key_queue, &kv, 0))
    {
        ESP_LOGW(TAG, "key send to queue fail...");
    }
}

void get_kvalye_queue_head(uint32_t *out_value)
{
    if(pdTRUE != xQueueReceive(key_queue, out_value, 0))
    {
        *out_value = 0;
        // ESP_LOGW(TAG, "key get queue head fail...");
    }
}

static void gpio_key_init(void *t, int32_t gpio_num, int8_t active_level)
{
    Button *cthis = (Button*)t;
    button_config_t btn_cfg;

    btn_cfg.type = BUTTON_TYPE_GPIO;
    btn_cfg.gpio_button_config.gpio_num = gpio_num;
    btn_cfg.gpio_button_config.active_level = active_level;

    cthis->button_handle = iot_button_create(&btn_cfg);
}

static void right_key_single_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);
    send_kvalue_to_queue_tail(KEY_VALUE_RIGHT);
}

static void right_key_long_start_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);
}
static void left_key_single_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);
    send_kvalue_to_queue_tail(KEY_VALUE_LEFT);
}
static void left_key_long_start_cb(void *args)
{
    uint8_t value = 0;

    // ESP_LOGI(TAG, "%s.", __func__);
    get_kvalye_queue_head(&value);
    ESP_LOGI(TAG, "Value is 0x%02X.", value);
}
static void up_single_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);
    send_kvalue_to_queue_tail(KEY_VALUE_UP);
}
static void up_long_start_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);
}
static void down_single_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);
    send_kvalue_to_queue_tail(KEY_VALUE_DOWN);
}
static void down_long_start_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);

    char buf[512];
    uint32_t heap_free_size = esp_get_free_heap_size();
    vTaskList(buf);
    
    printf("\r\nfree heap size : %d bytes.", heap_free_size);
    printf("\r\n TaskName     status priority LeftStack TaskNum\r\n");
    printf("%s", buf);
    printf("len = %d.\r\n", strlen(buf));
}
static void center_single_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);
    send_kvalue_to_queue_tail(KEY_VALUE_ENTER);
}
static void center_long_start_cb(void *args)
{
    // ESP_LOGI(TAG, "%s.", __func__);
    esp_restart();
}




/*********************************END OF FILE**********************************/
