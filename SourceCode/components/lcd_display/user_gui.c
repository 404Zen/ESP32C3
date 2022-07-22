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

static lv_group_t *g;
static lv_obj_t *tv;
static lv_obj_t *t1;
static lv_obj_t *t2;
static lv_obj_t *t3;

/* Private function prototypes -----------------------------------------------*/
static void home_btn_event_cb(lv_event_t *e);
static void set_btn_event_cb(lv_event_t *e);
static void devinfo_btn_event_cb(lv_event_t *e);

static void home_page(void);
static void home_set_date(lv_obj_t *_time, lv_obj_t *_date, lv_obj_t *_week);
static void home_set_weather(lv_obj_t *obj);
static void home_set_time(lv_obj_t *obj);
static void label_set_date(lv_obj_t *label);

static void device_info_page(void);

static void selectors_create(lv_obj_t *parent);
static void text_input_create(lv_obj_t *parent);
static void msgbox_create(void);

static void msgbox_event_cb(lv_event_t *e);
static void ta_event_cb(lv_event_t *e);

/* User code -----------------------------------------------------------------*/
/**
 * @brief  user_gui_task
 * @note   None.
 * @param  None.
 * @retval None.
 */
void user_gui_task(void)
{
#if 0
    home_scr = lv_obj_create(NULL);
    setting_scr = lv_obj_create(NULL);
    info_scr = lv_obj_create(NULL);
    // home_page();
#endif
    g = lv_group_create();
    lv_group_set_default(g);

    lv_indev_t *cur_drv = NULL;

    for (;;)
    {
        cur_drv = lv_indev_get_next(cur_drv);
        if (!cur_drv)
        {
            break;
        }

        if (cur_drv->driver->type == LV_INDEV_TYPE_KEYPAD)
        {
            lv_indev_set_group(cur_drv, g);
        }

        if (cur_drv->driver->type == LV_INDEV_TYPE_ENCODER)
        {
            lv_indev_set_group(cur_drv, g);
        }
    }

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 5);

    t1 = lv_tabview_add_tab(tv, "A");
    t2 = lv_tabview_add_tab(tv, "B");
    t3 = lv_tabview_add_tab(tv, "C");

    selectors_create(t1);
    text_input_create(t2);

    msgbox_create();
}

#if 0
/**
 * @brief  home_btn_event_cb
 * @note   None.
 * @param  None.
 * @retval None.
 */
static void home_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        home_page();
    }
}

static void set_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        lv_obj_t *label1 = lv_label_create(lv_scr_act());
        lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
        // lv_obj_align_to(label1, )
        lv_label_set_text(label1, "Setting Page");
    }
}

static void devinfo_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        device_info_page();
    }
}
#endif

#if 0
static void home_page(void)
{
    ESP_LOGI(TAG, "%s", __func__);
    lv_scr_load(home_scr);

    static lv_style_t style;
    lv_style_init(&style);

    lv_style_set_width(&style, 160);
    lv_style_set_height(&style, 80);

    lv_style_set_radius(&style, 0);
    lv_style_set_border_width(&style, 0);

    // lv_style_set_pad_ver(&style, 20);
    // lv_style_set_pad_left(&style, 5);

    lv_style_set_x(&style, lv_pct(0));
    lv_style_set_y(&style, 0);

    lv_style_set_bg_color(&style, lv_color_make(0x00, 0x00, 0x00));

    /*Create an object with the new style*/
    lv_obj_t *obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, &style, 0);
    lv_obj_center(obj);

    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_align_to(label1, )
    lv_label_set_text(label1, "Hello\nworld");

    lv_obj_t *home_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(home_btn, 5, 5);
    // lv_obj_center(button);
    lv_obj_set_pos(home_btn, 5, 5);
    // lv_obj_add_event_cb(home_btn, home_btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *set_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(set_btn, 5, 5);
    // // lv_obj_center(button);
    lv_obj_set_pos(set_btn, 15, 5);
    lv_obj_add_event_cb(set_btn, set_btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *info_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(info_btn, 5, 5);
    // // lv_obj_center(info_btn);
    lv_obj_set_pos(info_btn, 25, 5);
    lv_obj_add_event_cb(info_btn, devinfo_btn_event_cb, LV_EVENT_ALL, NULL);

    /* Group */
    lv_group_t *group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);

    lv_group_add_obj(group, home_btn);
    lv_group_add_obj(group, set_btn);
    lv_group_add_obj(group, info_btn);
}
#endif
static void device_info_page(void)
{
    ESP_LOGI(TAG, "%s", __func__);
    lv_scr_load(info_scr);

    lv_obj_t *info_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(info_btn, 5, 5);
    // // lv_obj_center(info_btn);
    lv_obj_set_pos(info_btn, 25, 5);
    // lv_obj_add_event_cb(info_btn, devinfo_btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *home_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(home_btn, 5, 5);
    // lv_obj_center(button);
    lv_obj_set_pos(home_btn, 5, 5);
    lv_obj_add_event_cb(home_btn, home_btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *set_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(set_btn, 5, 5);
    // // lv_obj_center(button);
    lv_obj_set_pos(set_btn, 15, 5);
    lv_obj_add_event_cb(set_btn, set_btn_event_cb, LV_EVENT_ALL, NULL);

    /* Group */
    lv_group_t *group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);

    lv_group_add_obj(group, info_btn);
    lv_group_add_obj(group, home_btn);
    lv_group_add_obj(group, set_btn);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void selectors_create(lv_obj_t *parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *obj;

    obj = lv_table_create(parent);
    lv_table_set_cell_value(obj, 0, 0, "00");
    lv_table_set_cell_value(obj, 0, 1, "01");
    lv_table_set_cell_value(obj, 1, 0, "10");
    lv_table_set_cell_value(obj, 1, 1, "11");
    lv_table_set_cell_value(obj, 2, 0, "20");
    lv_table_set_cell_value(obj, 2, 1, "21");
    lv_table_set_cell_value(obj, 3, 0, "30");
    lv_table_set_cell_value(obj, 3, 1, "31");
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    // obj = lv_calendar_create(parent);
    // lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    // obj = lv_btnmatrix_create(parent);
    // lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_checkbox_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_slider_create(parent);
    lv_slider_set_range(obj, 0, 10);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_switch_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_spinbox_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_dropdown_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    obj = lv_roller_create(parent);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_t *list = lv_list_create(parent);
    lv_obj_update_layout(list);
    if (lv_obj_get_height(list) > lv_obj_get_content_height(parent))
    {
        lv_obj_set_height(list, lv_obj_get_content_height(parent));
    }

    lv_list_add_btn(list, LV_SYMBOL_OK, "Apply");
    lv_list_add_btn(list, LV_SYMBOL_CLOSE, "Close");
    lv_list_add_btn(list, LV_SYMBOL_EYE_OPEN, "Show");
    lv_list_add_btn(list, LV_SYMBOL_EYE_CLOSE, "Hide");
    lv_list_add_btn(list, LV_SYMBOL_TRASH, "Delete");
    lv_list_add_btn(list, LV_SYMBOL_COPY, "Copy");
    lv_list_add_btn(list, LV_SYMBOL_PASTE, "Paste");
}

static void text_input_create(lv_obj_t *parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *ta1 = lv_textarea_create(parent);
    lv_obj_set_width(ta1, LV_PCT(100));
    lv_textarea_set_one_line(ta1, true);
    lv_textarea_set_placeholder_text(ta1, "Click with an encoder to show a keyboard");

    lv_obj_t *ta2 = lv_textarea_create(parent);
    lv_obj_set_width(ta2, LV_PCT(100));
    lv_textarea_set_one_line(ta2, true);
    lv_textarea_set_placeholder_text(ta2, "Type something");

    lv_obj_t *kb = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(ta1, ta_event_cb, LV_EVENT_ALL, kb);
    lv_obj_add_event_cb(ta2, ta_event_cb, LV_EVENT_ALL, kb);
}

static void msgbox_create(void)
{
    static const char *btns[] = {"Ok", "Cancel", ""};
    lv_obj_t *mbox = lv_msgbox_create(NULL, "Hi", "Welcome to the keyboard and encoder demo", btns, false);
    lv_obj_add_event_cb(mbox, msgbox_event_cb, LV_EVENT_ALL, NULL);
    lv_group_focus_obj(lv_msgbox_get_btns(mbox));
    lv_obj_add_state(lv_msgbox_get_btns(mbox), LV_STATE_FOCUS_KEY);
    lv_group_focus_freeze(g, true);

    lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *bg = lv_obj_get_parent(mbox);
    lv_obj_set_style_bg_opa(bg, LV_OPA_70, 0);
    lv_obj_set_style_bg_color(bg, lv_palette_main(LV_PALETTE_GREY), 0);
}

static void msgbox_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        const char *txt = lv_msgbox_get_active_btn_text(msgbox);
        if (txt)
        {
            lv_msgbox_close(msgbox);
            lv_group_focus_freeze(g, false);
            lv_group_focus_obj(lv_obj_get_child(t1, 0));
            lv_obj_scroll_to(t1, 0, 0, LV_ANIM_OFF);
        }
    }
}

static void ta_event_cb(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_get_act();
    if (indev == NULL)
        return;
    lv_indev_type_t indev_type = lv_indev_get_type(indev);

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED && indev_type == LV_INDEV_TYPE_ENCODER)
    {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(kb);
        lv_group_set_editing(lv_obj_get_group(kb), kb);
        lv_obj_set_height(tv, LV_VER_RES / 2);
        lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    }

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_height(tv, LV_VER_RES);
    }
}

/* ----- Startup Page ----- */
static const lv_coord_t startup_obj_width = 90;
static const lv_coord_t startup_obj_height = 40;
static lv_anim_timeline_t *anim_timeline = NULL;

static lv_obj_t *obj1 = NULL;
static lv_obj_t *obj2 = NULL;

static void set_width(void *var, int32_t v)
{
    lv_obj_set_width((lv_obj_t *)var, v);
}

static void set_height(void *var, int32_t v)
{
    lv_obj_set_height((lv_obj_t *)var, v);
}

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

    // lv_anim_t a2;
    // lv_anim_init(&a2);
    // lv_anim_set_var(&a2, obj1);
    // lv_anim_set_values(&a2, 0, obj_height);
    // lv_anim_set_early_apply(&a2, false);
    // lv_anim_set_exec_cb(&a2, (lv_anim_exec_xcb_t)set_height);
    // lv_anim_set_path_cb(&a2, lv_anim_path_ease_out);
    // lv_anim_set_time(&a2, 1000);

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

/* home page */
static lv_obj_t *home_label = NULL;     /* time */
static lv_obj_t *date_label = NULL;     /* date */
static lv_obj_t *weather_label = NULL;  /* weather */
static lv_obj_t *week_label = NULL;     /* week */
static lv_obj_t *tabview = NULL;

/* Show connecting... */
void connecting_timer(lv_timer_t *timer)
{
#if 0
    // ESP_LOGI(TAG, "%s", __func__);
    static uint8_t count = 0;

    count++;
    if (count > 3)
    {
        count = 0;
        lv_label_set_text(label, "CONNECTING");
    }
    else
    {
        lv_label_ins_text(label, LV_LABEL_POS_LAST, ".");
    }
#endif

    home_set_date(home_label, date_label, week_label);
}

void weather_timer_cb(lv_timer_t *timer)
{
    home_set_weather(weather_label);
}

static void tabview_event_callback(lv_event_t *e)
{
    lv_obj_t *tv = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    uint32_t key_value = 0;
    uint16_t target = 0;
    lv_tabview_t *tabview = (lv_tabview_t *)tv;

    printf("tab_cnt : %d. tab_cur : %d. tab_pos : %d.\r\n", tabview->tab_cnt, tabview->tab_cur, tabview->tab_pos);

    if (code == LV_EVENT_KEY)
    {
        key_value = lv_indev_get_key(indev_keypad);
        ESP_LOGI(TAG, "%s : lv_event_get_code %d. kv = %d.", __func__, code, key_value);

        if (key_value == LV_KEY_ENTER)
        {
            lv_obj_t *btn_matrix =  lv_tabview_get_tab_btns(tv);
            uint16_t id = lv_btnmatrix_get_selected_btn(btn_matrix);

            uint16_t state = lv_obj_get_state(btn_matrix);
            ESP_LOGI(TAG, "state = %d. id = %d.", state, id);
        }
        else if (key_value == LV_KEY_LEFT)
        {
            if(tabview->tab_cur <= 0)
            {
                target = tabview->tab_cnt - 1;
            }
            else
            {
                target = tabview->tab_cur - 1;
            }
        }
        else if(key_value == LV_KEY_RIGHT)
        {
            if(tabview->tab_cur >= tabview->tab_cnt - 1)
            {
                target = 0;
            }
            else
            {
                target = tabview->tab_cur + 1;
            }
        }
        lv_tabview_set_act(tv, target, LV_ANIM_ON);
    }
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
    lv_label_set_text(weather_label, "25°C / Sunny");
    lv_obj_set_pos(weather_label, 10, 10);

    lv_obj_t *text = lv_label_create(wireless_tab);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(text, lv_color_white(), 0);
    lv_obj_set_style_text_opa(text, LV_OPA_50, 0);
    lv_label_set_text(text, "WIRELESS");
    lv_obj_center(text);

    text = lv_label_create(other_tab);
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

    // lv_group_add_obj(group, other_tab);

    lv_scr_load_anim(par, LV_SCR_LOAD_ANIM_MOVE_TOP, 200, 0, true);
}

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

#if 0
static void home_set_time(lv_obj_t *obj)
{
    time_t now;
    struct tm timeinfo;

    // char strftime_buf[64];
    setenv("TZ", "CST-8", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    // ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);

    lv_label_set_text_fmt(obj, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

static void label_set_date(lv_obj_t *label)
{
    time_t now;
    struct tm timeinfo;

    setenv("TZ", "CST-8", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);

    lv_label_set_text_fmt(label, "%04d.%02d.%02d", timeinfo.tm_year+1900, timeinfo.tm_mon+1, timeinfo.tm_mday);
}
#endif

/*********************************END OF FILE**********************************/
