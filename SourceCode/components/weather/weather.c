/********************************************************************************************************
 * @file     weather.c
 *
 * @brief    weather c file
 *
 * @author   wujianping@nbdeli.com
 *
 * @date     2022-07-11
 *
 * @version  Ver: 0.1
 *
 * @attention
 *
 * HTTPS GET using plain mbedTLS sockets.
 *
 *******************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "zlib.h"
#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"

#include "weather.h"
#include "app_smartconfig.h"
#include "board_define.h"
#include "https_client.h"

#include "cJSON.h"

/* External variables --------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define TAG "WEATHER"

#define WEB_SERVER WEATHER_SERVER
#define WEB_PORT WEATHER_SERVER_PORT
#define WEB_URL WEATHER_SERVER_URL

#define WEATHER_LOCATION_ID_MAX_LEN 12
#define WEATHER_KEY_MAX_LEN 64
#define WEATHER_URL_MAX_LEN 256 /* 目前是 107 */
#define WEATHER_NVS_NAMESPACE "weather_data"
#define WEATHER_KEY_LOCATION_ID "weather_lid"
#define WEATHER_KEY_KEY "weather_key"

char weather_url[WEATHER_URL_MAX_LEN];
/* Private typedef -----------------------------------------------------------*/
#define WEATHER_EVT_REFRESH (1 << 0)
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
weather_data_t g_weather;
EventGroupHandle_t weather_evt_group;
TimerHandle_t weather_timer;

/* Private function prototypes -----------------------------------------------*/
static void weather_refresh_task(void *arg);
static int weather_response_decode(void *in_buf, size_t in_size, void *out_buf, size_t *out_size, size_t out_buf_size);
/* gzip data decompress */
static int network_gzip_decompress(void *in_buf, size_t in_size, void *out_buf, size_t *out_size, size_t out_buf_size);

static void set_weather_data(weather_data_t *data);

static void weather_timer_cb(TimerHandle_t timer);
/* User code -----------------------------------------------------------------*/
/**
 * @brief  weather_init
 * @note   None.
 * @param  None.
 * @retval None.
 */
void weather_init(void)
{
    char w_lid[WEATHER_LOCATION_ID_MAX_LEN];
    char w_key[WEATHER_KEY_MAX_LEN];
    ESP_LOGI(TAG, "%s", __func__);

    while (is_connect_to_ap() == false)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    memset(w_key, 0, WEATHER_KEY_MAX_LEN);
    memset(w_lid, 0, WEATHER_LOCATION_ID_MAX_LEN);

    if(get_weather_parameter(w_lid, w_key) != 0)
    {
        ESP_LOGE(TAG, "weather parameters is not set!");
    }
    else
    {
        /* Create weather application */
        sprintf(weather_url, "%s&location=%s&key=%s", WEB_URL, w_lid, w_key);
        ESP_LOGI(TAG, "Weather url %s", weather_url);

        weather_evt_group = xEventGroupCreate();
        if (weather_evt_group == NULL)
        {
            ESP_LOGI(TAG, "Weather event group create fail... will not start weather application");
        }
        else
        {
            weather_timer = xTimerCreate("xTimerCreate", pdMS_TO_TICKS(1000 * WEATHER_REFRESH_TIME), pdTRUE, NULL, weather_timer_cb);
            // weather_timer = xTimerCreate("xTimerCreate", pdMS_TO_TICKS(1000*10), pdTRUE, NULL, weather_timer_cb);
            if (weather_timer != NULL)
            {
                if (xTimerStart(weather_timer, 0) != pdPASS)
                {
                    ESP_LOGI(TAG, "Weather task timer create fail... will not start weather application");
                }
                else
                {
                    xTaskCreate(weather_refresh_task, "weather_task", 8192, NULL, 5, NULL);
                    vTaskDelay(pdTICKS_TO_MS(100));
                    xEventGroupSetBits(weather_evt_group, WEATHER_EVT_REFRESH);
                }
            }
            else
            {
                ESP_LOGI(TAG, "Weather task timer create fail... will not start weather application");
            }
        }
    }
}

/**
 * @brief  weather_refresh_task
 * @note   None.
 * @param  None.
 * @retval None.
 */
static void weather_refresh_task(void *arg)
{
    EventBits_t ux_bits;
    static unsigned char req_buf[1024];
    static size_t req_buf_len = 1024;

    char decode_buf[1024];
    size_t decode_len = 0, decode_buf_len = 1024;

    while (1)
    {
        ux_bits = xEventGroupWaitBits(weather_evt_group, WEATHER_EVT_REFRESH, pdTRUE, pdFALSE, portMAX_DELAY);

        if ((ux_bits & WEATHER_EVT_REFRESH) == WEATHER_EVT_REFRESH)
        {
            memset(req_buf, 0, sizeof(req_buf));
            req_buf_len = 1024; /* must */
            memset(decode_buf, 0, sizeof(decode_buf));
            decode_len = 0;
            https_get_request(WEB_SERVER, WEB_PORT, weather_url, req_buf, &req_buf_len);
            weather_response_decode(req_buf, req_buf_len, decode_buf, &decode_len, decode_buf_len);
#if 0
        for (size_t i = 0; i < decode_len; i++)
        {
            printf("%c", decode_buf[i]);
        }
        printf("\r\n");
#endif

            cJSON *root = cJSON_Parse(decode_buf);
            if (root != NULL)
            {
                ESP_LOGI(TAG, "json parse ok !");

                cJSON *weather = cJSON_GetObjectItem(root, "now");
                if (weather != NULL)
                {
                    cJSON *temp = cJSON_GetObjectItem(weather, "temp");
                    cJSON *text = cJSON_GetObjectItem(weather, "text");

                    weather_data_t data;
                    sprintf(data.temp, "%s", temp->valuestring);
                    sprintf(data.text, "%s", text->valuestring);
                    set_weather_data(&data);
                    ESP_LOGI(TAG, "weather is %s. %s", temp->valuestring, text->valuestring);
                }
            }
            cJSON_Delete(root);

            char *text = strstr(decode_buf, "text") + strlen("text");
            printf("is ... %d\r\n", text);

            printf("Minimum/free heap size: %d bytes / %d bytes\n", esp_get_minimum_free_heap_size(), esp_get_free_heap_size());
        }
    }
}

static void weather_timer_cb(TimerHandle_t timer)
{
    ESP_LOGI(TAG, "%s", __func__);
    xEventGroupSetBits(weather_evt_group, WEATHER_EVT_REFRESH);
}

static void set_weather_data(weather_data_t *data)
{
    // memset(&g_weather, 0, sizeof(weather_data_t));
    memcpy(&g_weather, data, sizeof(weather_data_t));
}

void get_weather_data(weather_data_t *data)
{
    memcpy(data, &g_weather, sizeof(weather_data_t));
}

/* 调用zlib解码 */
void response_data_process(char *data, size_t len)
{
    ESP_LOGI(TAG, "%s", __func__);
    size_t out_size = 1024;

#if 1
    for (size_t i = 0; i < len; i++)
    {
        putchar(*(data + i));
    }
    printf("\r\n");
#endif

    /* strstr 返回字符串s2第一次在s1中出现的位置 */
    char *resp_body = strstr(data, "\r\n\r\n") + strlen("\r\n\r\n"); /* 定位body位置 */

    /* 获取响应数据长度 */
    size_t resp_length;
    char *length_string = strcasestr(data, "Content-Length:");
    if (length_string)
    {
        sscanf(length_string, "%*s%d", &resp_length);
        ESP_LOGI(TAG, "body length %d.", resp_length);
    }
    else
    {
        ESP_LOGE(TAG, "unsupported chunked transfer encoding");
        goto exit;
    }

    /* 检查响应内容是否被gzip压缩 */
    uint8_t gzip_encoded = 0;
    char cotent_encoding_string[16];
    char *content_encoding_line = strcasestr(data, "Content-Encoding:");
    if (content_encoding_line)
    {
        sscanf(content_encoding_line, "%*s%s", cotent_encoding_string);
        ESP_LOGI(TAG, "%s", cotent_encoding_string);
        if (strcasestr(cotent_encoding_string, "gzip"))
        {
            gzip_encoded = 1;
            ESP_LOGI(TAG, " is encoding gzip.");
        }
    }

    // /* 判断内容是否过长 */
    // ESP_LOGI(TAG, "https response length: %d bytes", resp_length);
    // if(resp_length > out_buf_size-(resp_body-http_response)) {
    //     resp_length = out_buf_size-(resp_body-http_response);
    //     ESP_LOGW(TAG, "response too long, shrinking to %d bytes", resp_length);
    //     if(gzip_encoded) {
    //         ESP_LOGE(TAG, "gzip decode is not possible on shrinked buffer");
    //         goto exit;
    //     }
    // }

    if (gzip_encoded)
    { // gzip压缩后的响应内容
        ESP_LOGD(TAG, "gzip encoded response, decompressing...");

        char out_buf[1024];
        // size_t out_size = 1024;

        /* 解压请求内容 */
        int ret = network_gzip_decompress(resp_body, resp_length, out_buf, &out_size, 1024);
        if (ret != ESP_OK)
        {
            out_size = -1;
            ESP_LOGE(TAG, "gzip data decompression failed, code=%d", ret);
        }
        ESP_LOGD(TAG, "response size after decompression: %d bytes", out_size);

        printf("%s\r\n", out_buf);
    }
    else
    { //无压缩的响应内容
        ESP_LOGD(TAG, "no gzip encoded...");

        for (size_t i = 0; i < len; i++)
        {
            putchar(*(resp_body + i));
        }
        printf("\r\n");
        // out_size = resp_length;
    }

exit:
    return;
}

static int weather_response_decode(void *in_buf, size_t in_size, void *out_buf, size_t *out_size, size_t out_buf_size)
{
#if 0
    ESP_LOGD(TAG, "%s", __func__);
    for (size_t i = 0; i < in_size; i++)
    {
        putchar(*((char*)(in_buf + i)));
    }
    printf("\r\n");
#endif

    /* strstr 返回字符串s2第一次在s1中出现的位置 */
    void *resp_body = strstr(in_buf, "\r\n\r\n") + strlen("\r\n\r\n"); /* 定位body位置 */

    /* 获取响应数据长度 */
    size_t resp_length;
    char *length_string = strcasestr(in_buf, "Content-Length:");
    if (length_string)
    {
        sscanf(length_string, "%*s%d", &resp_length);
        ESP_LOGI(TAG, "body length %d.", resp_length);
    }
    else
    {
        ESP_LOGE(TAG, "unsupported chunked transfer encoding");
        // goto exit;
        return -1;
    }

    /* 检查响应内容是否被gzip压缩 */
    uint8_t gzip_encoded = 0;
    char cotent_encoding_string[16];
    char *content_encoding_line = strcasestr(in_buf, "Content-Encoding:");
    if (content_encoding_line)
    {
        sscanf(content_encoding_line, "%*s%s", cotent_encoding_string);
        ESP_LOGI(TAG, "%s", cotent_encoding_string);
        if (strcasestr(cotent_encoding_string, "gzip"))
        {
            gzip_encoded = 1;
            ESP_LOGI(TAG, "encoding gzip.");
        }
    }

    /* 判断内容是否过长 */
    ESP_LOGI(TAG, "https response length: %d bytes", resp_length);
    if (resp_length > out_buf_size - (resp_body - in_buf))
    {
        resp_length = out_buf_size - (resp_body - in_buf);
        ESP_LOGW(TAG, "response too long, shrinking to %d bytes", resp_length);
        if (gzip_encoded)
        {
            ESP_LOGE(TAG, "gzip decode is not possible on shrinked buffer");
            return -2;
        }
    }

    if (gzip_encoded)
    { // gzip压缩后的响应内容
        ESP_LOGD(TAG, "gzip encoded response, decompressing...");

        /* 解压请求内容 */
        int ret = network_gzip_decompress(resp_body, resp_length, out_buf, out_size, out_buf_size);
        if (ret != ESP_OK)
        {
            *out_size = -1;
            ESP_LOGE(TAG, "gzip data decompression failed, code=%d", ret);
        }
        ESP_LOGD(TAG, "response size after decompression: %d bytes", *out_size);
    }
    else
    { //无压缩的响应内容
        ESP_LOGD(TAG, "no gzip encoded...");

        memcpy(out_buf, resp_body, resp_length);
        *out_size = resp_length;
#if 0
        for (size_t i = 0; i < in_size; i++)
        {
            putchar(*((char *)(resp_body + i)));
        }
        printf("\r\n");
#endif
        // out_size = resp_length;
    }

    return 0;
}

static int network_gzip_decompress(void *in_buf, size_t in_size, void *out_buf, size_t *out_size, size_t out_buf_size)
{
    int err = 0;
    z_stream d_stream = {0}; /* decompression stream */
    d_stream.zalloc = NULL;
    d_stream.zfree = NULL;
    d_stream.opaque = NULL;
    d_stream.next_in = in_buf;
    d_stream.avail_in = 0;
    d_stream.next_out = out_buf;

    if ((err = inflateInit2(&d_stream, 47)) != Z_OK)
    {
        return err;
    }
    while (d_stream.total_out < out_buf_size - 1 && d_stream.total_in < in_size)
    {
        d_stream.avail_in = d_stream.avail_out = 1;
        if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END)
        {
            break;
        }
        if (err != Z_OK)
        {
            return err;
        }
    }

    if ((err = inflateEnd(&d_stream)) != Z_OK)
    {
        return err;
    }

    *out_size = d_stream.total_out;
    ((char *)out_buf)[*out_size] = '\0';

    return Z_OK;
}

/**
 * @brief  set_weather_parameter
 * @note   Set weather parameters to NVS.
 * @param  None.
 * @retval None.
**/
int set_weather_parameter(char *location_id, char *key)
{
    esp_err_t err;
    nvs_handle_t nvs_handler;
    size_t key_len = strlen(key);
    size_t id_len = strlen(location_id);

    if (key_len > WEATHER_KEY_MAX_LEN || id_len > WEATHER_LOCATION_ID_MAX_LEN)
    {
        return -2;
    }


    err = nvs_open(WEATHER_NVS_NAMESPACE, NVS_READWRITE, &nvs_handler);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Open nvs namespace fail.");
        return -1;
    }
    else
    {
        err = nvs_set_blob(nvs_handler, WEATHER_KEY_LOCATION_ID, location_id, id_len);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "Write weather location id to nvs fail; %s.", esp_err_to_name(err));
            ESP_ERROR_CHECK(nvs_commit(nvs_handler)); /* 提交NVS数据 */
            nvs_close(nvs_handler);

            return -1;
        }

        err = nvs_set_blob(nvs_handler, WEATHER_KEY_KEY, key, key_len);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "Write weather key to nvs fail; %s.", esp_err_to_name(err));
            ESP_ERROR_CHECK(nvs_commit(nvs_handler)); /* 提交NVS数据 */
            nvs_close(nvs_handler);

            return -1;
        }
    }

    ESP_ERROR_CHECK(nvs_commit(nvs_handler)); /* 提交NVS数据 */
    nvs_close(nvs_handler);

    /* set to ram */
    sprintf(weather_url, "%s&location=%s&key=%s", WEB_URL, location_id, key);
    ESP_LOGI(TAG, "Weather key update %s", weather_url);
    xEventGroupSetBits(weather_evt_group, WEATHER_EVT_REFRESH);

    return 0;
}

/**
 * @brief  get_weather_parameter
 * @note   get weather parameters from NVS.
 * @param  None.
 * @retval None.
**/
int get_weather_parameter(char *location_id, char *key)
{
    esp_err_t err;
    size_t key_len = WEATHER_KEY_MAX_LEN;
    size_t id_len = WEATHER_LOCATION_ID_MAX_LEN;
    nvs_handle_t nvs_handler;

    err = nvs_open(WEATHER_NVS_NAMESPACE, NVS_READWRITE, &nvs_handler);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Open nvs namespace fail.");
        return -1;
    }
    else
    {
        err = nvs_get_blob(nvs_handler, WEATHER_KEY_LOCATION_ID, location_id, &id_len);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "NVS get weather location id fail; %s.", esp_err_to_name(err));
            nvs_close(nvs_handler);

            return -1;
        }

        err = nvs_get_blob(nvs_handler, WEATHER_KEY_KEY, key, &key_len);
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "NVS get weather key fail; %s.", esp_err_to_name(err));
            nvs_close(nvs_handler);

            return -1;
        }
    }

    nvs_close(nvs_handler);
    return 0;
}


/*********************************END OF FILE**********************************/
