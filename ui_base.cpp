#include <lvgl.h>
extern const lv_font_t weather_icons;
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
lv_style_t style_bluetooth;
lv_style_t style_weather;
lv_style_t style_grid;
lv_style_t style_container;
lv_style_t style_title;
lv_style_t style_keyboard;

void init_style_keyboard()
{
    lv_style_init(&style_keyboard);

    /* Black background for keys */
    lv_style_set_bg_color(&style_keyboard, lv_color_black());
    lv_style_set_bg_opa(&style_keyboard, LV_OPA_COVER);

    /* Green border */
    lv_style_set_border_width(&style_keyboard, 1);
    lv_style_set_border_color(&style_keyboard, color_default);
    lv_style_set_border_opa(&style_keyboard, LV_OPA_COVER);

    /* Green text */
    lv_style_set_text_color(&style_keyboard, color_default);
}

void init_style_container()
{
    lv_style_init(&style_container);
    lv_style_set_text_font(&style_container, &lv_font_montserrat_16);
    lv_style_set_text_color(&style_container, color_default);
    lv_style_set_border_width(&style_container, 1);
    lv_style_set_border_color(&style_container, color_default);
    lv_style_set_bg_color(&style_container, lv_color_black());
    lv_style_set_pad_top(&style_container, 5);
    lv_style_set_pad_left(&style_container, 5);
    lv_style_set_pad_bottom(&style_container, 5);
    lv_style_set_pad_right(&style_container, 5);
    lv_style_set_margin_top(&style_container, 0);
    lv_style_set_margin_left(&style_container, 0);
    lv_style_set_margin_bottom(&style_container, 0);
    lv_style_set_margin_right(&style_container, 0);
}

void init_style_grid()
{
    lv_style_init(&style_grid);
    lv_style_set_text_font(&style_grid, &lv_font_montserrat_16);
    lv_style_set_text_color(&style_grid, color_default);
    lv_style_set_border_width(&style_grid, 0);
    lv_style_set_bg_color(&style_grid,  lv_color_black());
    lv_style_set_pad_row(&style_grid, 3);
    lv_style_set_pad_column(&style_grid, 0);
    lv_style_set_pad_top(&style_grid, 0);
    lv_style_set_pad_left(&style_grid, 0);
    lv_style_set_pad_bottom(&style_grid, 0);
    lv_style_set_pad_right(&style_grid, 0);
    lv_style_set_margin_top(&style_grid, 0);
    lv_style_set_margin_left(&style_grid, 0);
    lv_style_set_margin_bottom(&style_grid, 0);
    lv_style_set_margin_right(&style_grid, 0);
}

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
        { &style_wifi, &lv_font_montserrat_16, color_red, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_bluetooth, &lv_font_montserrat_16, color_red, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_weather, &weather_icons, color_default, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL },
        { &style_title, &lv_font_montserrat_24, color_default, LV_COLOR_TRANSP, LV_COLOR_TRANSP, NULL}
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
    init_style_grid();
    init_style_container();
    init_style_keyboard();
}

lv_obj_t *ui_add_aligned_label(char *cache_key, char *default_text, lv_obj_t *align_to_obj, 
    lv_style_t *style, align_cfg_t *aligns, size_cfg_t *sizes, lv_obj_t *screen)
{
    lv_obj_t *label = lv_label_create(screen);        /*Add a label the current screen*/
    if (cache_key)
        lv_label_set_text_fmt(label, get_string_key_value(cache_key, default_text).c_str());                 /*Set label text*/
    else
        lv_label_set_text(label, String(default_text).c_str());

    if (sizes != NULL) 
        lv_obj_set_size(label, sizes->width, sizes->height);

    if (aligns != NULL) {
        if (aligns->text_align != NULL)  
            lv_obj_set_style_text_align(label, aligns->text_align, 0);                                 /*Set center alignment*/
        if (align_to_obj)
            lv_obj_align_to(label, align_to_obj, aligns->align, aligns->x, aligns->y);
        else
            lv_obj_align(label, aligns->align, aligns->x, aligns->y);
    }
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
        lv_label_set_text(btn_label, get_string_key_value(cache_key, default_text).c_str());               /*Set the labels text*/
    else
        lv_label_set_text(btn_label, String(default_text).c_str());
    lv_obj_center(btn_label);
    lv_obj_add_style(btn_label, style, LV_PART_MAIN);

    return btn;
}

lv_obj_t *ui_add_title_label(char *label_text, lv_obj_t *screen)
{
    align_cfg_t title_label_align = {0, 40, LV_ALIGN_TOP_MID, LV_TEXT_ALIGN_AUTO};
    size_cfg_t title_label_size = {30, LV_HOR_RES};
    lv_obj_t *title_label = ui_add_aligned_label(NULL, label_text, NULL, &style_title, &title_label_align, 
        &title_label_size, screen);
    return title_label;
}

lv_obj_t *ui_add_content_container(int height, lv_obj_t *title, lv_obj_t *screen)
{
    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_set_size(container, LV_HOR_RES, height);
    lv_obj_add_style(container, &style_container, LV_PART_MAIN);
    lv_obj_set_scroll_dir(container, LV_DIR_VER);
    lv_obj_align_to(container, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    return container;
}

lv_obj_t *ui_add_button_row(lv_obj_t *screen)
{
    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_set_size(container, LV_HOR_RES, 40);
    lv_obj_add_style(container, &style_grid, LV_PART_MAIN);
    lv_obj_set_style_bg_color(container, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_color(container, color_default, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 1, LV_PART_MAIN);
    lv_obj_set_style_border_side(container, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, 0);

    return container;
}

lv_obj_t *ui_show_input_box(const char *title, const char *text_content, lv_obj_t *screen, lv_obj_t **target_textarea)
{
    /* Modal message box */
    lv_obj_t *mbox = lv_msgbox_create(screen);                                            
    //lv_msgbox_add_text(mbox, "Password:");                     
    lv_obj_set_width(mbox, 220);
    /* Content area of msgbox */
    lv_msgbox_add_text(mbox, title);
    lv_obj_t *content = lv_msgbox_get_content(mbox);                  
    lv_obj_add_style(mbox, &style_container, LV_PART_MAIN);
    lv_obj_add_style(mbox, &style_default_small, LV_PART_MAIN);
    /* Text area inside msgbox */
    lv_obj_t *textarea = lv_textarea_create(content);   
    lv_obj_add_style(textarea, &style_container, LV_PART_MAIN);                              
    lv_textarea_set_one_line(textarea, true);                               
    lv_obj_set_width(textarea, lv_pct(100));   
    lv_textarea_set_text(textarea, text_content);      
    *target_textarea = textarea;
    lv_obj_align(mbox, LV_ALIGN_TOP_MID, 0, 40);
    return mbox;
}

static void confirm_box_event_cb(lv_event_t * e) {
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *btn_container = lv_obj_get_parent(btn);
    lv_obj_t *mbox = lv_obj_get_parent(btn_container);
    void (*user_cb)(lv_event_t *) = (void (*)(lv_event_t *))lv_event_get_user_data(e);

    if (user_cb) 
        user_cb(e);

    lv_msgbox_close(mbox);
}

static void confirm_box_cancel_event_cb(lv_event_t * e)
{
    lv_obj_t *mbox = (lv_obj_t *)lv_event_get_user_data(e);
    lv_msgbox_close(mbox);
}

lv_obj_t *ui_show_confirm_box(const char *title, void (*callback)(lv_event_t *), const char *confirm_text, lv_obj_t *screen)
{
    /* Modal message box */
    lv_obj_t *mbox = lv_msgbox_create(screen);                                            
                    
    lv_obj_set_width(mbox, 230);
    /* Content area of msgbox */
    lv_msgbox_add_text(mbox, title);
    lv_obj_t *content = lv_msgbox_get_content(mbox);                  
    lv_obj_add_style(mbox, &style_container, LV_PART_MAIN);
    lv_obj_add_style(mbox, &style_default_small, LV_PART_MAIN);
    /* Text area inside msgbox */

    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style, lv_color_black());
    lv_style_set_bg_opa(&btn_style, LV_OPA_COVER);
    lv_style_set_border_color(&btn_style, color_default);  // Border in default_color
    lv_style_set_border_width(&btn_style, 3);              // Border thickness (px)
    lv_style_set_border_opa(&btn_style, LV_OPA_COVER);
    

    lv_obj_t *confirm_btn = lv_msgbox_add_footer_button(mbox, confirm_text);
    lv_obj_add_event_cb(confirm_btn, confirm_box_event_cb, LV_EVENT_CLICKED, (void*)callback);
    lv_obj_add_style(confirm_btn, &btn_style, 0);  // Main part, default state
    lv_obj_t *confirm_label = lv_obj_get_child(confirm_btn, 0);
    lv_obj_add_style(confirm_label, &style_default_small, 0);

    lv_obj_t *cancel_btn = lv_msgbox_add_footer_button(mbox, "Cancel");
    lv_obj_add_event_cb(cancel_btn, confirm_box_cancel_event_cb, LV_EVENT_CLICKED, mbox);  // Pass mbox to close
    lv_obj_add_style(cancel_btn, &btn_style, 0);  // Main part, default state
    lv_obj_t *cancel_label = lv_obj_get_child(cancel_btn, 0);
    lv_obj_add_style(cancel_label, &style_default_small, 0);

    return mbox;    
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

lv_obj_t *ui_create_grid(const int32_t *col_dsc, const int32_t *row_dsc, grid_row_t *rows, int num_rows, lv_obj_t *screen)
{
    lv_obj_t *grid = lv_obj_create(screen);
    lv_obj_add_style(grid, &style_grid, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(grid, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(grid, row_dsc, 0);
    lv_obj_set_size(grid, 240, 240);
    lv_obj_align(grid, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);

    for(int i = 0; i < num_rows; i++) {
        lv_obj_t *label = lv_label_create(grid);
        lv_label_set_text(label, rows[i].title);
        lv_obj_set_grid_cell(label, LV_GRID_ALIGN_STRETCH, 0, 1,
            LV_GRID_ALIGN_STRETCH, i, 1);
        if (rows[i].value) {
            lv_obj_t *value_label = lv_label_create(grid);
            *rows[i].value = value_label;
            lv_obj_set_grid_cell(value_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                LV_GRID_ALIGN_STRETCH, i, 1);
        }
    }

    return grid;
}