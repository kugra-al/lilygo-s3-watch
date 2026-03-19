#include "ui_screens.h"
lv_obj_t *gps_satellites_label, *gps_latitude_label, *gps_longitude_label, *gps_altitude_label, *gps_status_label;

void gps_sync_confirm_btn_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        const char *latitude_label = lv_label_get_text(gps_latitude_label);
        const char *longitude_label = lv_label_get_text(gps_longitude_label);
        // Need to write to settings
        return;
    }
}

void gps_sync_button_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        const char *latitude_label = lv_label_get_text(gps_latitude_label);
        const char *longitude_label = lv_label_get_text(gps_longitude_label);
        if (lv_strcmp(latitude_label, GPS_DEFAULT_TEXT) != 0 && lv_strcmp(longitude_label, GPS_DEFAULT_TEXT) != 0) {
            ui_show_confirm_box("Sync with settings?", gps_sync_confirm_btn_cb, "Sync", lv_scr_act());
        }
    }
}

void gps_refresh_button_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (monitor.gps_enabled)
            update_gps_stats(1000);
    }
}

void gps_toggle_button_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        toggle_gps();
    }
}

void draw_gps_screen()
{
    Serial.println("Drawing gps screen");
    lv_obj_t *screen = screens[GPS_SCREEN];
    lv_obj_t *gps_title_label = ui_add_title_label("GPS", screen);

    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, gps_title_label, screen);    

    static int32_t col_dsc[] = {140, 80, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {20, 20, 20, 20, 20, LV_GRID_TEMPLATE_LAST};

    static grid_row_t rows[] = {
        {"Satellites", &gps_satellites_label},
        {"Latitude", &gps_latitude_label},
        {"Longitude", &gps_longitude_label},
        {"Altitude", &gps_altitude_label},
        {"Status", &gps_status_label}
    };
    lv_obj_t *grid = ui_create_grid(col_dsc, row_dsc, rows, 5, content);

    lv_label_set_text(gps_satellites_label, GPS_DEFAULT_TEXT);
    lv_label_set_text(gps_latitude_label, GPS_DEFAULT_TEXT);
    lv_label_set_text(gps_longitude_label, GPS_DEFAULT_TEXT);
    lv_label_set_text(gps_altitude_label, GPS_DEFAULT_TEXT);
    lv_label_set_text(gps_status_label, monitor.gps_enabled ? LV_SYMBOL_OK : LV_SYMBOL_CLOSE);

    lv_obj_t *btn_container = ui_add_button_row(screen);
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *sync_gps_btn = ui_add_button(NULL, LV_SYMBOL_DOWNLOAD, NULL, &style_default_small, gps_sync_button_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_MID;
    lv_obj_t *refresh_gps_btn = ui_add_button(NULL, LV_SYMBOL_REFRESH, NULL, &style_default_small, gps_refresh_button_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_RIGHT;
    lv_obj_t *toggle_gps_btn = ui_add_button(NULL, LV_SYMBOL_POWER, NULL, &style_default_small, gps_toggle_button_cb, 
        &btn_align, &btn_size, btn_container);

}

void toggle_gps()
{
    if (monitor.gps_enabled) {
        gps_enabled = false;
        hw_update_monitor();
    } else {
        gps_enabled = true;
        hw_update_monitor();
        update_gps_stats(1000);
    }
    lv_label_set_text(gps_status_label, monitor.gps_enabled ? LV_SYMBOL_OK : LV_SYMBOL_CLOSE);
}

void update_gps_stats(unsigned long ms)
{
    if (monitor.gps_enabled) {
        unsigned long start = millis();
        do {
            // read message from GPSSerial
            while (SerialGPS.available()) {
                int r = SerialGPS.read();
                instance.gps.encode(r);
                Serial.write(r);
            }
        } while (millis() - start < ms);
        lv_label_set_text_fmt(gps_satellites_label, "%d", instance.gps.satellites.isValid() ? instance.gps.satellites.value() : 0);
        lv_label_set_text_fmt(gps_latitude_label, "%.5f", instance.gps.location.isValid() ? instance.gps.location.lat() : 0);
        lv_label_set_text_fmt(gps_longitude_label, "%.5f", instance.gps.location.isValid() ? instance.gps.location.lng() : 0);
        lv_label_set_text_fmt(gps_altitude_label, "%.2fm", instance.gps.altitude.isValid() ? instance.gps.altitude.meters() : 0);
    }
}