#include "ui_screens.h"
#include "ui_screen_cache.h"
#include "ui_base.h"
#include "cache.h"

lv_obj_t *cache_used_label, *cache_free_label, *cache_total_label, *cache_namespace_label;

void clear_cache_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        nvs_full_reset();
    }
    return;
}

void clear_cache_btn_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ui_show_confirm_box("Clear Cache?", clear_cache_event_cb, "Clear Cache", lv_screen_active());
    }
}

void update_cache_stats()
{
    nvs_stats_t stats;
    nvs_get_stats(NULL, &stats);

    lv_label_set_text_fmt(cache_used_label, "%d", stats.used_entries);
    lv_label_set_text_fmt(cache_free_label, "%d", stats.free_entries);
    lv_label_set_text_fmt(cache_total_label, "%d", stats.total_entries);
    lv_label_set_text_fmt(cache_namespace_label, "%d", stats.namespace_count);
}

void draw_cache_screen()
{
    Serial.println("Drawing cache screen");
    lv_obj_t *screen = secondary_screens[CACHE_SCREEN];
    lv_obj_t *cache_title_label = ui_add_title_label("Cache", screen);

    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, cache_title_label, screen);

    static int32_t col_dsc[] = {140, 80, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {20, 20, 20, 20, LV_GRID_TEMPLATE_LAST};

    static grid_row_t rows[] = {
        {"Used", &cache_used_label},
        {"Free", &cache_free_label},
        {"Total", &cache_total_label},
        {"Namespaces", &cache_namespace_label}
    };
    lv_obj_t *grid = ui_create_grid(col_dsc, row_dsc, rows, 4, content);
    update_cache_stats();
    lv_obj_t *btn_container = ui_add_button_row(screen);
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *settings_btn = ui_add_button(NULL, "Back", NULL, &style_default_small, back_button_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_RIGHT;
    lv_obj_t *clear_cache_btn = ui_add_button(NULL, "Clear", NULL, &style_default_small, clear_cache_btn_cb, 
        &btn_align, &btn_size, btn_container);

}