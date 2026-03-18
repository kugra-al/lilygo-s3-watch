#include "ui_screens.h"
lv_obj_t *gps_satellites_label, *gps_latitude_label, *gps_longitude_label, *gps_altitude_label;

void draw_gps_screen()
{
    Serial.println("Drawing gps screen");
    lv_obj_t *screen = screens[GPS_SCREEN];
    lv_obj_t *gps_title_label = ui_add_title_label("GPS", screen);

    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT, gps_title_label, screen);    

    static int32_t col_dsc[] = {140, 80, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {20, 20, 20, 20, LV_GRID_TEMPLATE_LAST};

    static grid_row_t rows[] = {
        {"Satellites", &gps_satellites_label},
        {"Latitude", &gps_latitude_label},
        {"Longitude", &gps_longitude_label},
        {"Altitude", &gps_altitude_label}
    };
    lv_obj_t *grid = ui_create_grid(col_dsc, row_dsc, rows, 4, content);
    update_gps_stats(1000);
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