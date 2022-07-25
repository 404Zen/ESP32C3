/********************************************************************************************************
 * @file     user_gui.c
 *
 * @brief    user_gui c file
 *
 * @author   wujianping@nbdeli.com
 *
 * @date     2022-07-06
 *
 * @version  Ver: 0.1
 *
 * @attention
 *
 * None.
 *
 *******************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "user_gui.h"
#include "lcd_display.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

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

#include "weather.h"

/* External variables --------------------------------------------------------*/
extern lv_indev_t *indev_keypad;
/* Private define ------------------------------------------------------------*/
#define TAG "USER_GUI"
/* Private typedef -----------------------------------------------------------*/
const char *weekstr[7] = {
    "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sta"
};
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
lv_obj_t *home_scr;
lv_obj_t *setting_scr;
lv_obj_t *info_scr;

/* Private function prototypes -----------------------------------------------*/
static void home_page(void);
static void home_set_date(lv_obj_t *_time, lv_obj_t *_date, lv_obj_t *_week);
static void home_set_weather(lv_obj_t *obj);



/************************************************************
 * Startup Page
************************************************************/
static const lv_coord_t startup_obj_width = 90;
static const lv_coord_t startup_obj_height = 40;
static lv_anim_timeline_t *anim_timeline = NULL;

static lv_obj_t *obj1 = NULL;
static lv_obj_t *obj2 = NULL;

#if 1
static void set_width(void *var, int32_t v)
{
    lv_obj_set_width((lv_obj_t *)var, v);
}

static void set_height(void *var, int32_t v)
{
    lv_obj_set_height((lv_obj_t *)var, v);
}
#endif


static void anim_done_cb(lv_anim_t *anim)
{
    /* Load home page after startup anim */
    ESP_LOGI(TAG, "%s", __func__);
    lv_anim_del_all();
    home_page();
}

static void anim_timeline_create(void)
{
    /* obj1 */
    lv_anim_t a1;
    lv_anim_init(&a1);
    lv_anim_set_var(&a1, obj1);
    lv_anim_set_values(&a1, 0, startup_obj_width);
    lv_anim_set_early_apply(&a1, false);
    lv_anim_set_exec_cb(&a1, (lv_anim_exec_xcb_t)set_width);
    lv_anim_set_path_cb(&a1, lv_anim_path_overshoot);
    lv_anim_set_time(&a1, 1000);

    /* obj2 */
    lv_anim_t a3;
    lv_anim_init(&a3);
    lv_anim_set_var(&a3, obj2);
    lv_anim_set_values(&a3, 0, startup_obj_width);
    lv_anim_set_early_apply(&a3, false);
    lv_anim_set_exec_cb(&a3, (lv_anim_exec_xcb_t)set_width);
    lv_anim_set_path_cb(&a3, lv_anim_path_step);
    lv_anim_set_ready_cb(&a3, anim_done_cb);
    lv_anim_set_time(&a3, 1000);

    /* Create anim timeline */
    anim_timeline = lv_anim_timeline_create();
    lv_anim_timeline_add(anim_timeline, 0, &a1);
    // lv_anim_timeline_add(anim_timeline, 0, &a2);
    lv_anim_timeline_add(anim_timeline, 200, &a3);

    ESP_LOGI(TAG, "Startup page...");
    lv_anim_timeline_start(anim_timeline);
}

void user_startup_page(void)
{
    static lv_style_t par_style;
    lv_style_init(&par_style);
    lv_style_set_radius(&par_style, 0);
    lv_style_set_bg_opa(&par_style, LV_OPA_COVER);
    lv_style_set_bg_color(&par_style, lv_color_make(0x00, 0x00, 0x00));

    lv_obj_t *par = lv_scr_act();
    // lv_obj_t * par = lv_obj_create(lv_scr_act());
    // lv_obj_remove_style_all(par);
    lv_obj_set_size(par, LV_HOR_RES, LV_VER_RES);
    lv_obj_add_style(par, &par_style, 0);

    /* create objects */
    obj1 = lv_obj_create(par);
    lv_obj_remove_style_all(obj1);
    lv_obj_set_size(obj1, startup_obj_width, startup_obj_height);
    lv_obj_set_style_border_color(obj1, lv_color_make(0xFF, 0x00, 0x00), 0);
    lv_obj_set_style_border_side(obj1, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(obj1, 3, 0);
    lv_obj_set_style_border_post(obj1, true, 0);
    lv_obj_center(obj1);

    obj2 = lv_label_create(obj1);
    lv_obj_set_style_text_font(obj2, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(obj2, lv_color_white(), 0);
    lv_label_set_text(obj2, "HELLO");
    lv_obj_center(obj2);

    anim_timeline_create();
    lv_scr_load(par);
}

/************************************************************
 * Home page variables
************************************************************/
static lv_obj_t *home_label = NULL;     /* time */
static lv_obj_t *date_label = NULL;     /* date */
static lv_obj_t *weather_label = NULL;  /* weather */
static lv_obj_t *week_label = NULL;     /* week */
static lv_obj_t *tabview = NULL;

/************************************************************
 * Wireless page variables
************************************************************/
static lv_obj_t *wifi_ssid_label = NULL;
static lv_obj_t *wifi_ip_label = NULL;

/* Show connecting... */
void connecting_timer(lv_timer_t *timer)
{
    home_set_date(home_label, date_label, week_label);
}

void weather_timer_cb(lv_timer_t *timer)
{
    home_set_weather(weather_label);
}


/* tabview btn_martix event callback */
static void tv_btns_event_callback(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if(event_code == LV_EVENT_KEY)
    {
        // uint32_t key_value = 0;
        lv_obj_t *btn_matrix = lv_event_get_target(e);
        // lv_event_code_t code = lv_event_get_code(e);
        // key_value = lv_indev_get_key(indev_keypad);
        uint16_t id = lv_btnmatrix_get_selected_btn(btn_matrix);
        lv_tabview_set_act(tabview, id, LV_ANIM_ON);
    }
}

/**
 * @brief  home_page
 * @note   主界面以及创建各个TAB.
 * @param  None.
 * @retval None.
**/
static void home_page(void)
{
    static lv_style_t par_style;
    lv_style_init(&par_style);
    lv_style_set_radius(&par_style, 0);
    lv_style_set_bg_opa(&par_style, LV_OPA_COVER);
    lv_style_set_bg_color(&par_style, lv_color_make(0x00, 0x00, 0x00));

    // lv_obj_t* par = lv_scr_act();
    lv_obj_t *par = lv_obj_create(NULL);
    // lv_obj_remove_style_all(par);
    lv_obj_set_size(par, LV_HOR_RES, LV_VER_RES);
    lv_obj_add_style(par, &par_style, 0);

    /* 在顶部创建一个tabview */
    tabview = lv_tabview_create(par, LV_DIR_TOP, 1);
    lv_obj_set_style_bg_color(tabview, lv_color_make(0x00, 0x00, 0x00), 0);

    // lv_obj_add_style(tv, &tv_style, 0);
    // lv_obj_set_style_bg_color(tabview, lv_color_make(0x00, 0x00, 0x00), 0);
    // lv_obj_set_style_border_width(tabview, 0, 0);

    /* 创建TAB */
    lv_obj_t *home_tab = lv_tabview_add_tab(tabview, " ");
    lv_obj_t *wireless_tab = lv_tabview_add_tab(tabview, " ");
    lv_obj_t *other_tab = lv_tabview_add_tab(tabview, " ");

    /* tab btn 样式 */
    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_border_width(tab_btns, 5, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_color(tab_btns, lv_color_make(0xFF, 0x00, 0x00), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_border_opa(tab_btns, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_CHECKED);

    /* tab内容 */
    home_label = lv_label_create(home_tab);
    lv_obj_set_style_text_font(home_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(home_label, lv_color_white(), 0);
    lv_obj_set_style_text_opa(home_label, LV_OPA_50, 0);
    lv_label_set_text(home_label, "CONNECTING");
    lv_obj_center(home_label);

    date_label = lv_label_create(home_tab);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(date_label, lv_color_white(), 0);
    lv_obj_set_style_text_opa(date_label, LV_OPA_50, 0);
    lv_label_set_text(date_label, "1970.08.01");
    lv_obj_set_pos(date_label, 90, 65);

    week_label = lv_label_create(home_tab);
    lv_obj_set_style_text_font(week_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(week_label, lv_color_white(), 0);
    lv_obj_set_style_text_opa(week_label, LV_OPA_50, 0);
    lv_label_set_text(week_label, "Tues");
    lv_obj_set_pos(week_label, 10, 65);

    weather_label = lv_label_create(home_tab);
    lv_obj_set_style_text_font(weather_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(weather_label, lv_color_white(), 0);
    lv_obj_set_style_text_opa(weather_label, LV_OPA_50, 0);
    lv_label_set_text(weather_label, "25°C");
    lv_obj_set_pos(weather_label, 10, 10);

    wifi_ssid_label = lv_label_create(wireless_tab);
    lv_obj_set_style_text_font(wifi_ssid_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(wifi_ssid_label, lv_color_white(), 0);
    lv_obj_set_style_text_opa(wifi_ssid_label, LV_OPA_50, 0);
    lv_label_set_text(wifi_ssid_label, "SSID:AP_SSID");
    lv_obj_set_pos(wifi_ssid_label, 10, 10);
    // lv_obj_center(text);

    wifi_ip_label = lv_label_create(wireless_tab);
    lv_obj_set_style_text_font(wifi_ip_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(wifi_ip_label, lv_color_white(), 0);
    lv_obj_set_style_text_opa(wifi_ip_label, LV_OPA_50, 0);
    lv_label_set_text(wifi_ip_label, "IP:192.168.100.100");
    lv_obj_set_pos(wifi_ip_label, 10, 30);

    lv_obj_t *text = lv_label_create(other_tab);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(text, lv_color_white(), 0);
    lv_obj_set_style_text_opa(text, LV_OPA_50, 0);
    lv_label_set_text(text, "Comming soon...");
    lv_obj_center(text);

    lv_timer_t *conn_timer = lv_timer_create(connecting_timer, 1000, NULL);
    lv_timer_t *weather_timer = lv_timer_create(weather_timer_cb, 3000, NULL);

    /* indev group */
    lv_group_t *group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);

    
    #if 1   /* Use btn_matrix in tabview... OK */
    lv_obj_t *btn_matrix =  lv_tabview_get_tab_btns(tabview);
    lv_group_add_obj(group, btn_matrix);
    lv_obj_add_event_cb(btn_matrix, tv_btns_event_callback, LV_EVENT_KEY, NULL);
    #endif

    #if 0
    lv_obj_add_event_cb(tabview, tabview_event_callback, LV_EVENT_KEY, NULL);
    lv_group_add_obj(group, tabview);
    #endif

    lv_scr_load_anim(par, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 0, true);
}


/************************************************************
 * Home page funcion
************************************************************/
static void home_set_date(lv_obj_t *_time, lv_obj_t *_date, lv_obj_t *_week)
{
    time_t now;
    struct tm timeinfo;

    // char strftime_buf[64];
    setenv("TZ", "CST-8", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);

    if(_time != NULL)
    {
        lv_label_set_text_fmt(_time, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }

    if(_date != NULL)
    {
        lv_label_set_text_fmt(_date, "%04d.%02d.%02d", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday);
    }

    if(_week != NULL)
    {
        lv_label_set_text_fmt(_week, "%s", weekstr[timeinfo.tm_wday]);
    }
}

static void home_set_weather(lv_obj_t *obj)
{
    weather_data_t data;
    static uint8_t count = 0;
    count++;
    get_weather_data(&data);
    if(obj != NULL)
    {
        if(count % 2)
        {
            lv_label_set_text_fmt(obj, "%s°C", data.temp);
        }
        else
        {
            lv_label_set_text_fmt(obj, "%s", data.text);
        }
    }
}

/************************************************************
 * Wireless page funcion
************************************************************/
void wl_set_ssid_label(char *ssid)
{
    if(wifi_ssid_label != NULL)
        lv_label_set_text_fmt(wifi_ssid_label, "SSID : %s", ssid);
}

void wl_set_ip_label(char *ip)
{
    if(wifi_ip_label != NULL)
        lv_label_set_text_fmt(wifi_ip_label, "IP : %s", ip);
}


/*********************************END OF FILE**********************************/
