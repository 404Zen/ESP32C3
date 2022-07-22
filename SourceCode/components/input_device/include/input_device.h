/********************************************************************************************************
* @file     input_device.h
* 
* @brief    input_device header file
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
#ifndef __INPUT_DEVICE_H__
#define __INPUT_DEVICE_H__

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

#define KEY_VALUE_DOWN                  0x01
#define KEY_VALUE_UP                    0x02
#define KEY_VALUE_LEFT                  0x03
#define KEY_VALUE_RIGHT                 0x04
#define KEY_VALUE_ENTER                 0x05
/* Exported functions prototypes ---------------------------------------------*/
void input_btn_init(void);
void get_kvalye_queue_head(uint32_t *out_value);



#endif /* __INPUT_DEVICE_H__ */
/*********************************END OF FILE**********************************/
