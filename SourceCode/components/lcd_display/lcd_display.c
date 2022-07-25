/********************************************************************************************************
 * @file     lcd_display.c
 *
 * @brief    lcd_display c file
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lcd_display.h"
#include "board_define.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_freertos_hooks.h"
#include "esp_system.h"

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "lvgl_helpers.h"
// #include "lv_color.h"

/* For lvgl 7 */
// #include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"
// #include "lv_examples/src/lv_demo_keypad_encoder/lv_demo_keypad_encoder.h"
// #include "lv_examples/src/lv_demo_benchmark/lv_demo_benchmark.h"
// #include "lv_examples/src/lv_demo_stress/lv_demo_stress.h"

#include "../lvgl/demos/lv_demos.h"
#include "../lvgl/demos/stress/lv_demo_stress.h"

#include "input_device.h"
#include "user_gui.h"

/* Private define ------------------------------------------------------------*/
#define TAG "LV_DEMO"

#define LV_TICK_PERIOD_MS 1

#ifdef CONFIG_IDF_TARGET_ESP32C3
#define LCD_HOST SPI2_HOST
#define PIN_NUM_MISO GPIO_NUM_NC
#define PIN_NUM_MOSI LCD_SDA
#define PIN_NUM_CLK LCD_SCL
#define PIN_NUM_CS LCD_CS

#define PIN_NUM_DC LCD_DC
#define PIN_NUM_RST LCD_RES
#define PIN_NUM_BL LCD_BL
#endif

// To speed up transfers, every SPI transfer sends a bunch of lines. This define specifies how many. More means more memory use,
// but less overhead for setting up / finishing transfers. Make sure 240 is dividable by this.
#define PARALLEL_LINES 16
#define LCD_WIDTH 160
#define LCD_HEIGTH 80

// #define MY_DISP_HOR_RES                 80
// #define MY_DISP_VER_RES                 160

#define MY_DISP_HOR_RES 160
#define MY_DISP_VER_RES 80
/* Private typedef -----------------------------------------------------------*/
/*
 The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct.
*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
lv_indev_t *indev_keypad;

/* Private function prototypes -----------------------------------------------*/
static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void create_demo_application(void);

static void keypad_init(void);
static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
static uint32_t keypad_get_key(void);

/* User code -----------------------------------------------------------------*/
/**
 * @brief  funtion_brief
 * @note   None.
 * @param  None.
 * @retval None.
 */
void lcd_display_init(void)
{
    /* If you want to use a task to create the graphic, you NEED to create a Pinned task
     * Otherwise there can be problem such as memory corruption and so on.
     * NOTE: When not using Wi-Fi nor Bluetooth you can pin the guiTask to core 0 */
    xTaskCreatePinnedToCore(guiTask, "gui", 4096 * 2, NULL, 0, NULL, 0);
    // xTaskCreate(guiTask, "guiTask", 4096*2, NULL, (configMAX_PRIORITIES-2), NULL);
}

#define BUF_SIZE 20
/* Creates a semaphore to handle concurrent call to lvgl stuff
 * If you wish to call *any* lvgl function from other threads/tasks
 * you should lock on the very same semaphore! */
SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter)
{
    static lv_indev_drv_t indev_drv;
    
    (void)pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    // lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    // assert(buf1 != NULL);

    // lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    // assert(buf2 != NULL);

    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2_1[MY_DISP_HOR_RES * BUF_SIZE];                                /*A buffer for 10 rows*/
    static lv_color_t buf_2_2[MY_DISP_HOR_RES * BUF_SIZE];                                /*An other buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * BUF_SIZE); /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_driver_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf_dsc_2;

    lv_disp_drv_register(&disp_drv);

#if 0 /* LVGL 7 */
    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);
#endif

    /* Indev device */
    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));
    

    /* Create the demo application */
    ESP_LOGI(TAG, "Demo Start!");
    create_demo_application();
    
    while (1)
    {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
        {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }
    }

    ESP_LOGI(TAG, "Out of except!");
    /* A task should NEVER return */
    free(buf_2_1);
    free(buf_2_2);

    vTaskDelete(NULL);
}

static void create_demo_application(void)
{
    user_startup_page();

    /* Wait for system start up */
    
}

static void lv_tick_task(void *arg)
{
    (void)arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void keypad_init(void)
{
    input_btn_init();
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;

    /*Get the current x and y coordinates*/
    // mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    if(act_key != 0) {
        data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
            // case KEY_VALUE_DOWN:
            case KEY_VALUE_RIGHT:
                // act_key = LV_KEY_NEXT;
                act_key = LV_KEY_RIGHT;
                break;
            // case KEY_VALUE_UP:
            case KEY_VALUE_LEFT:
                // act_key = LV_KEY_PREV;
                act_key = LV_KEY_LEFT;
                break;
            // case KEY_VALUE_LEFT:
            case KEY_VALUE_UP:
                // act_key = LV_KEY_LEFT;
                act_key = LV_KEY_UP;
                break;
            // case KEY_VALUE_RIGHT:
            case KEY_VALUE_DOWN:
                // act_key = LV_KEY_RIGHT;
                act_key = LV_KEY_DOWN;
                break;
            case KEY_VALUE_ENTER:
                act_key = LV_KEY_ENTER;
                break;
        }

        last_key = act_key;
        // ESP_LOGI(TAG, "Press key is %d.", act_key);
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    uint32_t key_value = 0;
    get_kvalye_queue_head(&key_value);

    return key_value;
}

/*********************************END OF FILE**********************************/
