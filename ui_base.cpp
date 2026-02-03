#include <lvgl.h>
#include "ui_base.h"
#include "cache.h"
#include "ui_screens.h"

lv_style_t style_default;           
lv_style_t style_default_medium;
lv_style_t style_default_large;
lv_style_t style_default_small;
lv_style_t style_connected;
lv_style_t style_inactive;
lv_style_t style_disconnected;
lv_style_t style_roller;
lv_style_t style_roller_selected;
lv_style_t style_charge;
lv_style_t style_wifi;

void init_styles()
{
    typedef struct {
        lv_style_t      *style;  
        const lv_font_t *font;
        lv_color_t      color;
        lv_color_t      background_color;
        lv_color_t      border_color;
        int             border_width;
    } style_cfg_t;

    static style_cfg_t styles[] = {
        // Style, font, font color, bg color, border color, border width
        { &style_default, &lv_font_montserrat_20, color_default, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_default_large, &lv_font_montserrat_36, color_default, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_default_medium, &lv_font_montserrat_18, color_default, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_default_small, &lv_font_montserrat_16, color_default, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_connected, &lv_font_montserrat_16, color_green, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_inactive, &lv_font_montserrat_16, color_grey, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_disconnected, &lv_font_montserrat_16, color_red, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_roller, &lv_font_montserrat_16, color_default, lv_color_black(), color_default, 1},
        { &style_roller_selected, &lv_font_montserrat_18, lv_color_black(), color_default, lv_color_black(), 1},
        { &style_charge, &lv_font_montserrat_16, color_yellow, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_wifi, &lv_font_montserrat_16, color_red, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL }
    };
    
    for (int i = 0; i < sizeof(styles) / sizeof(styles[0]); i++) {
        lv_style_init(styles[i].style);
        lv_style_set_text_font(styles[i].style, styles[i].font);
        lv_style_set_text_color(styles[i].style, styles[i].color);
        if (styles[i].border_width) {
            lv_style_set_bg_color(styles[i].style, styles[i].background_color);
            lv_style_set_border_color(styles[i].style, styles[i].border_color);
            lv_style_set_border_width(styles[i].style, styles[i].border_width);
        }
    }
}

lv_obj_t *ui_add_aligned_label(char *cache_key, char *default_text, lv_obj_t *align_to_obj, 
    lv_style_t *style, align_cfg_t *aligns, size_cfg_t *sizes, lv_obj_t *screen)
{
    lv_obj_t *label = lv_label_create(screen);        /*Add a label the current screen*/
    if (cache_key)
        lv_label_set_text_fmt(label, getStringKV(cache_key, default_text).c_str());                 /*Set label text*/
    else
        lv_label_set_text(label, String(default_text).c_str());

    if (sizes != NULL) {
        lv_obj_set_width(label, sizes->width);
        lv_obj_set_height(label, sizes->height);
    }
    if (aligns->text_align != NULL)  
        lv_obj_set_style_text_align(label, aligns->text_align, 0);                                 /*Set center alignment*/
    if (align_to_obj)
        lv_obj_align_to(label, align_to_obj, aligns->align, aligns->x, aligns->y);
    else
        lv_obj_align(label, aligns->align, aligns->x, aligns->y);
    lv_obj_add_style(label, style, LV_PART_MAIN);
    return label;
}

lv_obj_t *ui_add_button(char *cache_key, char *default_text, lv_obj_t *align_to_obj, 
    lv_style_t *style, void (*callback)(lv_event_t *), align_cfg_t *aligns, size_cfg_t *sizes, lv_obj_t *screen)
{
    lv_obj_t *btn = lv_button_create(screen);            /*Add a button the current screen*/
    lv_obj_set_size(btn, sizes->width, sizes->height);                          /*Set its size*/
    lv_obj_add_event_cb(btn, callback, LV_EVENT_ALL, NULL);  /*Assign a callback to the button*/
    if (align_to_obj)
        lv_obj_align_to(btn, align_to_obj, aligns->align, aligns->x, aligns->y);
    else
        lv_obj_align(btn, aligns->align, aligns->x, aligns->y); /*Set the label to it and align it in the center below the label*/

    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style, lv_color_black());
    lv_style_set_bg_opa(&btn_style, LV_OPA_COVER);
    lv_style_set_border_color(&btn_style, color_default);  // Border in default_color
    lv_style_set_border_width(&btn_style, 3);              // Border thickness (px)
    lv_style_set_border_opa(&btn_style, LV_OPA_COVER);
    lv_obj_add_style(btn, &btn_style, 0);  // Main part, default state

    lv_obj_t *btn_label = lv_label_create(btn);           /*Add a label to the button*/
    if (cache_key)
        lv_label_set_text(btn_label, getStringKV(cache_key, default_text).c_str());               /*Set the labels text*/
    else
        lv_label_set_text(btn_label, String(default_text).c_str());
    lv_obj_center(btn_label);
    lv_obj_add_style(btn_label, style, LV_PART_MAIN);

    return btn;
}

lv_obj_t *ui_add_title_label(char *label_text, lv_obj_t *screen)
{
    align_cfg_t title_label_align = {0, 45, LV_ALIGN_TOP_MID, LV_TEXT_ALIGN_AUTO};
    size_cfg_t title_label_size = {50, 180};
    lv_obj_t *title_label = ui_add_aligned_label(NULL, label_text, NULL, &style_default_large, &title_label_align, 
        &title_label_size, screen);
    return title_label;
}

lv_obj_t *init_popup(char *label_text, char *btn_text, void (*callback)(lv_event_t *))
{
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *label = lv_label_create(popup);        /*Add a label the current screen*/
    lv_label_set_text(label, String(label_text).c_str()); 
    lv_obj_add_style(label, &style_default_small, LV_PART_MAIN);
    if (callback) {
        align_cfg_t btn_align = {50, 40, LV_ALIGN_BOTTOM_MID, LV_TEXT_ALIGN_AUTO};
        size_cfg_t btn_size = {35, 100};
        ui_add_button(NULL, btn_text, label, &style_default_small, callback, &btn_align, &btn_size, popup);
    }
    return popup;
}
