/********************************************************************************************************
* @file     https_client.c
* 
* @brief    https_client c file
* 
* @author   wujianping@nbdeli.com
* 
* @date     2022-07-12 
* 
* @version  Ver: 0.1 
* 
* @attention 
* 
* None.
* 
*******************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "https_client.h"

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "esp_crt_bundle.h"

#include "zlib.h"
#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"

#include "app_smartconfig.h"
#include "weather.h"

/* External variables --------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define TAG         "HTTPS_C"
/* Private typedef -----------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* User code -----------------------------------------------------------------*/
 /**
  * @brief  https_get_request
  * @note   无证书认证.
  * @param  server:服务器地址
  * @param  port:端口
  * @param  url:请求地址
  * @param  out_buf:输出数据指针
  * @param  buf_len:[IN/OUT]输出数据长度
  * @retval None.
  */
int https_get_request(char *server, char *port, char *url, unsigned char *out_buf, size_t *buf_len)
{
    int ret;
    // int len;
    char get_req_buf[1024];
    memset(get_req_buf, 0, sizeof(get_req_buf));
    sprintf(get_req_buf, "GET %s  HTTP/1.0\r\nHost: %s\r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n", url, server);


    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_ssl_config_init(&conf);
    mbedtls_entropy_init(&entropy);

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     NULL, 0)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ctr_drbg_seed returned %d", ret);
        // abort();
        goto exit;
    }

    ESP_LOGI(TAG, "Attaching the certificate bundle...");

    ret = esp_crt_bundle_attach(&conf);
    if (ret < 0)
    {
        ESP_LOGE(TAG, "esp_crt_bundle_attach returned -0x%x\n\n", -ret);
        // abort();
        goto exit;
    }

    ESP_LOGI(TAG, "Setting hostname for TLS session...");
    /* Hostname set here should match CN in server certificate */
    if ((ret = mbedtls_ssl_set_hostname(&ssl, server)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_set_hostname returned -0x%x", -ret);
        // abort();
        goto exit;
    }


    ESP_LOGI(TAG, "Setting up the SSL/TLS structure...");
    if ((ret = mbedtls_ssl_config_defaults(&conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_config_defaults returned %d", ret);
        goto exit;
    }

    /* MBEDTLS_SSL_VERIFY_OPTIONAL is bad for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.

       You should consider using MBEDTLS_SSL_VERIFY_REQUIRED in your own code.
    */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
    // mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#ifdef CONFIG_MBEDTLS_DEBUG
    mbedtls_esp_enable_debug_log(&conf, CONFIG_MBEDTLS_DEBUG_LEVEL);
#endif

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_ssl_setup returned -0x%x\n\n", -ret);
        goto exit;
    }

    mbedtls_net_init(&server_fd);
    ESP_LOGI(TAG, "Connecting to %s:%s...", server, port);

    if ((ret = mbedtls_net_connect(&server_fd, server, port, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        ESP_LOGE(TAG, "mbedtls_net_connect returned -%x", -ret);
        goto exit;
    }
    ESP_LOGI(TAG, "Connected.");

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
    
    ESP_LOGI(TAG, "Performing the SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            ESP_LOGE(TAG, "mbedtls_ssl_handshake returned -0x%x", -ret);
            goto exit;
        }
    }

    ESP_LOGI(TAG, "Writing HTTP request...");
    size_t written_bytes = 0;
    do
    {
        ret = mbedtls_ssl_write(&ssl,
                                (const unsigned char *)get_req_buf + written_bytes,
                                strlen(get_req_buf) - written_bytes);
        if (ret >= 0)
        {
            ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        }
        else if (ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != MBEDTLS_ERR_SSL_WANT_READ)
        {
            ESP_LOGE(TAG, "mbedtls_ssl_write returned -0x%x", -ret);
            goto exit;
        }
    } while (written_bytes < strlen(get_req_buf));

    ESP_LOGI(TAG, "Reading HTTP response...");
    bzero(out_buf, *buf_len);
    do
    {
        // len = *buf_len - 1;
        ret = mbedtls_ssl_read(&ssl, out_buf, (*buf_len - 1));

        printf("%s, %d, ret = 0x%X.\r\n", __FILE__, __LINE__, ret);

        /* 如果返回的数据长度超过buf_len, 将会导致前面的数据被覆盖。 */
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;

        if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
        {
            ret = 0;
            break;
        }

        if (ret < 0)
        {
            ESP_LOGE(TAG, "mbedtls_ssl_read returned -0x%x", -ret);
            break;
        }

        if (ret == 0)
        {
            ESP_LOGI(TAG, "connection closed");
            break;
        }
        
        *buf_len = ret;
        ESP_LOGI(TAG, "%d bytes read", ret);


    #if 0       /* 数据处理 */
        /* Print response directly to stdout as it is read */
        for (int i = 0; i < ret; i++)
        {
            // putchar(buf[i]);
            putchar(*((char*)(out_buf+i)));
        }
    #else
        // response_data_process(out_buf, ret);
    #endif

    } while (1);

    mbedtls_ssl_close_notify(&ssl);

exit:
    mbedtls_ssl_session_reset(&ssl);
    mbedtls_ssl_free(&ssl);
    mbedtls_net_free(&server_fd);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    // mbedtls_x509_crt_free(&cacert);

    if (ret != 0)
    {
        mbedtls_strerror(ret, (char *)out_buf, 100);
        ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, out_buf);
    }
    // putchar('\n'); // JSON output doesn't have a newline at end
    printf("Minimum/free heap size: %d bytes / %d bytes\n", esp_get_minimum_free_heap_size(), esp_get_free_heap_size());
    // printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    return 0;
}









/*********************************END OF FILE**********************************/
