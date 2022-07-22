/********************************************************************************************************
 * @file     ledc_fade.c
 *
 * @brief    ledc_fade c file
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
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_err.h"

#include "driver/ledc.h"

#include "ledc_fade.h"
#include "board_define.h"

/* External variables --------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
#define LEDC_LS_TIMER LEDC_TIMER_1
#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE

#define LEDC_LS_CH0_GPIO LED_D4
#define LEDC_LS_CH0_CHANNEL LEDC_CHANNEL_0

#define LEDC_TEST_CH_NUM (1)
#define LEDC_TEST_DUTY (4000)           /* 和定时器设置的位数有关，默认13bit, full range = 8192, 4000接近一半的占空比 */
#define LEDC_TEST_FADE_TIME (2000)
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
SemaphoreHandle_t counting_sem;

/* 定义LEDC的每个通道 */
ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
    {.channel = LEDC_LS_CH0_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_LS_CH0_GPIO,
     .speed_mode = LEDC_LS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_LS_TIMER,
     .flags.output_invert = 0},
};

/* Private function prototypes -----------------------------------------------*/
static bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg);
static void ledc_fade_task(void *arg);
/* User code -----------------------------------------------------------------*/
/**
 * @brief  ledc_fade_init
 * @note   None.
 * @param  None.
 * @retval None.
 */
void ledc_fade_init(void)
{
    int ch;

    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = 5000,                      // frequency of PWM signal
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .timer_num = LEDC_LS_TIMER,           // timer index
        .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);

    // Set LED Controller with previously prepared configuration
    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
    {
        ledc_channel_config(&ledc_channel[ch]);
    }

    // Initialize fade service.
    ledc_fade_func_install(0);
    ledc_cbs_t callbacks = {
        .fade_cb = cb_ledc_fade_end_event};

    counting_sem = xSemaphoreCreateCounting(LEDC_TEST_CH_NUM, 0);

    for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
    {
        ledc_cb_register(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, &callbacks, (void *)counting_sem);
    }

    xTaskCreate(ledc_fade_task, "ledc_fade_task", 1024, NULL, 0, NULL);
}

/*
 * This callback function will be called when fade operation has ended
 * Use callback only if you are aware it is being called inside an ISR
 * Otherwise, you can use a semaphore to unblock tasks
 */
static bool cb_ledc_fade_end_event(const ledc_cb_param_t *param, void *user_arg)
{
    portBASE_TYPE taskAwoken = pdFALSE;

    if (param->event == LEDC_FADE_END_EVT)
    {
        SemaphoreHandle_t counting_sem = (SemaphoreHandle_t)user_arg;
        xSemaphoreGiveFromISR(counting_sem, &taskAwoken);
    }

    return (taskAwoken == pdTRUE);
}

static void ledc_fade_task(void *arg)
{
    size_t ch = 0;
    while (1)
    {
        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
        {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                                    ledc_channel[ch].channel, LEDC_TEST_DUTY, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                            ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }

        for (int i = 0; i < LEDC_TEST_CH_NUM; i++)
        {
            xSemaphoreTake(counting_sem, portMAX_DELAY);
        }

        for (ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
        {
            ledc_set_fade_with_time(ledc_channel[ch].speed_mode,
                                    ledc_channel[ch].channel, 0, LEDC_TEST_FADE_TIME);
            ledc_fade_start(ledc_channel[ch].speed_mode,
                            ledc_channel[ch].channel, LEDC_FADE_NO_WAIT);
        }

        for (int i = 0; i < LEDC_TEST_CH_NUM; i++)
        {
            xSemaphoreTake(counting_sem, portMAX_DELAY);
        }
    }
}

/*********************************END OF FILE**********************************/
