/********************************************************************************************************
* @file     https_client.h
* 
* @brief    https_client header file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HTTPS_CLIENT_H__
#define __HTTPS_CLIENT_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

/* Private includes ----------------------------------------------------------*/

/* Exported defines ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
int https_get_request(char *server, char *port, char *url,unsigned char *out_buf, size_t *buf_len);




#endif /* __HTTPS_CLIENT_H__ */
/*********************************END OF FILE**********************************/
