#include "core/lv_obj_pos.h"
#include "HardwareSerial.h"
#include "ArduinoJson/Array/JsonArray.hpp"
#include "misc/lv_area.h"
#include <lvgl.h>
#include <LilyGoLib.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "cache.h"
#include "ui_base.h"
#include "ui_screens.h"
#include "config.h"
#include "watch.h"
#include "hw_monitor.h"


lv_obj_t *time_label, *time_label_2;
lv_obj_t *date_label, *clock_temp_label, *clock_wind_label, *current_weather, *sun_status;
lv_obj_t *wifi_label, *battery_label, *charge_label, *bluetooth_label, *gps_label, *alarm_symbol_label;
lv_obj_t *status_ssid_value_label, *status_local_ip_value_label, *status_gateway_ip_value_label, 
    *status_power_value_label, *status_temp_value_label;
lv_obj_t *alarm_time_label, *alarm_hours_roller, *alarm_minutes_roller;
lv_obj_t *popup;
lv_obj_t *weather_screen_label, *weather_screen_status_label;

int current_screen = CLOCK_SCREEN;
alarm_cfg_t ui_alarm = {0, 0, false, false, 0};

static void clock_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        if (current_screen == CLOCK_SCREEN) 
            switch_to_screen(ALARM_SCREEN);
    }
}

static void alarm_stop_btn_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        alarm_stop();
    }
}

void alarm_start()
{
    ui_alarm.running = true;
    ui_alarm.end_time = millis()+ONE_MINUTE;
    init_popup("Alarm", "Stop", alarm_stop_btn_cb);
}

void alarm_stop()
{
    ui_alarm.end_time = 0;
    ui_alarm.running = false;
    ui_alarm.set = false;
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
}

void alarm_alert() {
    instance.drv.setWaveform(0, 113);
    instance.drv.run();
}

void update_date() 
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        lv_label_set_text(date_label, getStringKV("date", "Date: --").c_str());
        return;
    }
    static lv_obj_t *battery_label;
    char date[20];
    size_t len = strftime(date, sizeof(date), "%a. %Y-%m-%d", &timeinfo);
    String date_str = (String)date;
    putStringKV("date", date_str);
    lv_label_set_text(date_label, date_str.c_str());  // "Mon 2026-01-05"
}

void update_time() 
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        if (current_screen == CLOCK_SCREEN) {
            lv_label_set_text(time_label, getStringKV("time", "--:--:--").c_str());
            lv_label_set_text(time_label_2, getStringKV("time_2", "--:--:--").c_str());
        } else {
            lv_label_set_text(time_label_2, getStringKV("time", "--:--:--").c_str());
        }
        return;
    }
    char time_cacheBuf[10];
    snprintf(time_cacheBuf, sizeof(time_cacheBuf),
        (const char*)"%02d:%02d:%02d",
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        
    time_t now = mktime(&timeinfo);
    time_t time2 = now + (TIME_2_OFFSET * 3600);
    struct tm time2info;
    localtime_r(&time2, &time2info);
    char time_2_cacheBuf[10];
    snprintf(time_2_cacheBuf, sizeof(time_2_cacheBuf),
        (const char*)"%02d:%02d:%02d",
        time2info.tm_hour, time2info.tm_min, time2info.tm_sec);

    putStringKV("time", time_cacheBuf);
    putStringKV("time_2", time_2_cacheBuf);
    // Use real RTC time
    if (current_screen == CLOCK_SCREEN) { // On clock screen, show time_2 as the offset
        lv_label_set_text(time_label, time_cacheBuf);
        lv_label_set_text(time_label_2, time_2_cacheBuf);
    }
    else
        lv_label_set_text(time_label_2, time_cacheBuf);
    if (ui_alarm.set) {
        if (!ui_alarm.running) {
            if (timeinfo.tm_hour == ui_alarm.hour && timeinfo.tm_min == ui_alarm.minute) {
                alarm_start();
            }
        } else {
            if (ui_alarm.end_time-millis() >= ONE_MINUTE) {
                alarm_stop();
            } else {
                alarm_alert();
            }
        }
    }
}

String wind_dir_to_text(float deg) {
  // normalize
  if (deg < 0) deg += 360;
  if (deg >= 360) deg -= 360;

  static const char* dirs[] = {"N","NE","E","SE","S","SW","W","NW","N"};
  int index = (int)round(deg / 45.0);   // 360 / 8 = 45°
  return String(dirs[index]);
}

char *get_weather_icon(int code) 
{
    if (code >= 0 && code <= 1)
        return "N"; // Sunny
    if (code >= 2 && code <= 3)
        return "C"; // Partly cloudy
    if (code >= 4 && code <= 48)
        return "P"; // Cloudy
    if (code >= 51 && code <= 55 || code == 61 || code >= 80 && code <= 81)
        return "M"; // Drizzle
    if (code >= 56 && code <= 57 || code >= 66 && code <= 71 || code >= 85 && code <= 86)
        return "L"; // Freezing drizzle/light snow
    if (code >= 63 && code <= 65 || code == 82)
        return "B"; // Rain
    if (code >= 73 && code <= 77)
        return "D"; // Snow
    if (code == 95)
        return "A"; // Thunderstorm
    if (code >= 96 && code <= 99)
        return "K"; // Thunderstorm with rain
    return "J";
}

void update_weather()
{
    Serial.println("Attempting to fetch weather");
    String url = String("https://api.open-meteo.com/v1/forecast?latitude=")+LATITUDE+
        "&longitude="+LONGITUDE+"&current_weather=true&daily=sunrise,sunset,weather_code,temperature_2m_max,"+
        "temperature_2m_min&timezone=Europe/Vilnius&forecast_days=14";  
    HTTPClient http;
    http.begin(url);
    int code = http.GET();
    if (code == 200) {
        Serial.println("Fetched weather");
        String payload = http.getString();
        StaticJsonDocument<2048> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
            float temp = doc["current_weather"]["temperature"];
            float wind = doc["current_weather"]["windspeed"];
            String windDir = wind_dir_to_text(doc["current_weather"]["winddirection"]); 
            String sunrise = doc["daily"]["sunrise"][0];
            String sunset = doc["daily"]["sunset"][0];
            int weatherCode = doc["current_weather"]["weathercode"];
           // String weatherIcon = get_weather_icon(weatherCode);
            int tPos = sunrise.indexOf('T');
            sunrise = sunrise.substring(tPos+1, tPos+6);
            sunset = sunset.substring(tPos+1, tPos+6);
            Serial.println("Temp: " + String(temp) + "°C");
            
            lv_label_set_text_fmt(clock_temp_label, "%.1f°C", temp);
            lv_label_set_text_fmt(clock_wind_label, "%.1fkm/h %s", wind, windDir.c_str());
            lv_label_set_text_fmt(current_weather, "%s", String(get_weather_icon(weatherCode)).c_str());   

            char cacheBuf[24];
            snprintf(cacheBuf, sizeof(cacheBuf),
                "Rise: %s Set: %s", sunrise, sunset);
            putStringKV("suntimes", cacheBuf);
            lv_label_set_text(sun_status, cacheBuf);
            JsonArray timeArr = doc["daily"]["time"];
            JsonArray tempMinArr = doc["daily"]["temperature_2m_min"];
            JsonArray tempMaxArr = doc["daily"]["temperature_2m_max"];
            JsonArray codeArr = doc["daily"]["weather_code"];
           // lv_obj_add_flag(weather_screen_label, LV_OBJ_FLAG_HIDDEN);
            align_cfg_t weather_align = {0, 25, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
            align_cfg_t weather_icon_align = {25, 0, LV_ALIGN_RIGHT_MID, LV_TEXT_ALIGN_AUTO};
            char weatherBuf[512];
            char weatherStatusBuf[64];
            for (int i = 0; i < 7; i++) {
                String timeStr = String(timeArr[i]);
                float tempMin = tempMinArr[i].as<float>();
                float tempMax = tempMaxArr[i].as<float>();
                char *code = get_weather_icon(codeArr[i].as<int>());
                weather_align.y += 20;
                char textBuf[64];
                char statusBuf[4];
                snprintf(textBuf, sizeof(textBuf), "%s %.0fc/%.0fc\n", timeStr.c_str(), tempMax, tempMin);
                strcat(weatherBuf, textBuf);
                snprintf(statusBuf, sizeof(statusBuf), "%s\n", String(code).c_str());
                strcat(weatherStatusBuf, statusBuf);
                //lv_obj_t *weather_date = ui_add_aligned_label(NULL, textBuf, NULL, &style_default_medium, &weather_align, NULL, screens[WEATHER_SCREEN]);
                //lv_obj_t *weather_symbol = ui_add_aligned_label(NULL, code, weather_date, &style_weather, &weather_icon_align, NULL, screens[WEATHER_SCREEN]);
            }
            lv_label_set_text(weather_screen_label, String(weatherBuf).c_str());
            lv_label_set_text(weather_screen_status_label, String(weatherStatusBuf).c_str());
        }
    }
    http.end();
}

void ui_refresh_sensor_labels()
{
    lv_label_set_text_fmt(status_ssid_value_label, "%s", monitor.ssid);
    lv_label_set_text_fmt(status_local_ip_value_label, "%s", monitor.local_ip);
    lv_label_set_text_fmt(status_gateway_ip_value_label, "%s", monitor.gateway_ip);
    char battery_cacheBuf[20];
    float volts = monitor.battery_voltage / 1000.0f;
    snprintf(battery_cacheBuf, sizeof(battery_cacheBuf),
         "%d%% %d.%02dv",
         monitor.battery_percent, 
         (int)volts,
         (int)(volts * 100) % 100);
    lv_label_set_text(status_power_value_label, battery_cacheBuf);
    char temp_cacheBuf[20];
    snprintf(temp_cacheBuf, sizeof(temp_cacheBuf),
        "%dc",
        (int)monitor.temperature);
    lv_label_set_text(status_temp_value_label, temp_cacheBuf);
}

void refresh_screen_headers()
{
    Serial.println("Refreshing screen headers");
    char battery_percent_cacheBuf[8];
    snprintf(battery_percent_cacheBuf, sizeof(battery_percent_cacheBuf), "%d%%", monitor.battery_percent);
    lv_label_set_text(battery_label, battery_percent_cacheBuf);
    if (monitor.wifi_connected)
        lv_style_set_text_color(&style_wifi, color_green);
    else
        lv_style_set_text_color(&style_wifi, color_red);
    int charge_adjust = 0;
    if (monitor.battery_percent >= 100)
        charge_adjust = -5;
    lv_obj_align_to(charge_label, battery_label, LV_ALIGN_TOP_RIGHT, -40+charge_adjust, 0);
    if (monitor.charging) {
        lv_obj_clear_flag(charge_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(wifi_label, charge_label, LV_ALIGN_TOP_RIGHT, -15, 0);
    } else {
        lv_obj_add_flag(charge_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(wifi_label, battery_label, LV_ALIGN_TOP_RIGHT, -40+charge_adjust, 0);
    }
    // These need to be realigned if wifi label changes position
    lv_obj_align_to(bluetooth_label, wifi_label, LV_ALIGN_TOP_RIGHT, -25, 0);
    lv_obj_align_to(gps_label, bluetooth_label, LV_ALIGN_TOP_RIGHT, -20, 0);
    if (ui_alarm.set)
        lv_obj_add_style(alarm_symbol_label, &style_connected, LV_PART_MAIN);
    else
        lv_obj_add_style(alarm_symbol_label, &style_inactive, LV_PART_MAIN);
    lv_obj_align_to(alarm_symbol_label, gps_label, LV_ALIGN_TOP_RIGHT, -20, 0);
}

void draw_screen_headers()
{
    Serial.println("Drawing screen headers");
    lv_obj_t *header = lv_obj_create(lv_layer_top());
    lv_obj_set_size(header, LV_HOR_RES, 40);
    lv_obj_set_style_bg_color(header, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);

    static align_cfg_t aligns = {0, 0, LV_ALIGN_TOP_RIGHT, LV_TEXT_ALIGN_AUTO};

    battery_label = ui_add_aligned_label(NULL, "??%", NULL, &style_default_small, &aligns, NULL, header);

    aligns.x = -40;
    charge_label = ui_add_aligned_label(NULL, LV_SYMBOL_CHARGE, battery_label, &style_charge, &aligns, NULL, header);

    aligns.x = -30;
    wifi_label = ui_add_aligned_label(NULL, LV_SYMBOL_WIFI, battery_label, &style_wifi, &aligns, NULL, header);

    aligns.x = -10;
    bluetooth_label = ui_add_aligned_label(NULL, LV_SYMBOL_BLUETOOTH, wifi_label, &style_inactive, &aligns, NULL, header);
    gps_label = ui_add_aligned_label(NULL, LV_SYMBOL_GPS, bluetooth_label, &style_inactive, &aligns, NULL, header);
    alarm_symbol_label = ui_add_aligned_label(NULL, LV_SYMBOL_BELL, gps_label, &style_inactive, &aligns, NULL, header);
    aligns = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    time_label_2 = ui_add_aligned_label("time_2", "--:--:--", NULL, &style_default_small, &aligns, NULL, header);
    refresh_screen_headers();

    popup = lv_obj_create(lv_layer_top());
    lv_obj_set_size(popup, 200, 80); 
    lv_obj_set_style_bg_color(popup, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(popup, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(popup, color_default, LV_PART_MAIN);  // Border in default_color
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align_to(popup, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
}

void alarm_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
        /*Get the first child of the button which is the label */
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        char hoursBuf[8];
        char minutesBuf[8];
        char timeBuf[16];
        lv_roller_get_selected_str(alarm_hours_roller, hoursBuf, sizeof(hoursBuf));
        lv_roller_get_selected_str(alarm_minutes_roller, minutesBuf, sizeof(minutesBuf));
        snprintf(timeBuf, sizeof(timeBuf), "Alarm time: %s:%s", hoursBuf, minutesBuf);
        lv_label_set_text(alarm_time_label, timeBuf);
        ui_alarm.hour = atoi(hoursBuf);
        ui_alarm.minute = atoi(minutesBuf);
        ui_alarm.set = true;
        put_int_key_value("ui_alarm_hour", ui_alarm.hour);
        put_int_key_value("ui_alarm_min", ui_alarm.minute);
        put_bool_key_value("ui_alarm_set", ui_alarm.set);
    }
}

void alarm_cancel_btn_event_cb(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
        ui_alarm.set = false;  
        put_bool_key_value("ui_alarm_set", ui_alarm.set);
    }  
}

void alarm_time_change_event_handler(lv_event_t *e) 
{
   return;
}

void draw_alarm_screen()
{
    Serial.println("Drawing alarm screen");
    lv_obj_t *screen = screens[ALARM_SCREEN];
    lv_obj_t *alarm_title_label = ui_add_title_label("Alarm", screen);
    alarm_hours_roller = lv_roller_create(screen);
    //lv_obj_set_size(alarm_hours_roller, 60, 80);
    static char hour_opts[200];
    for(int i = 0; i < 23; i++) {
        sprintf(hour_opts + strlen(hour_opts), "%02d\n", i);
    }
    strcat(hour_opts, "23"); // Add seperate to remove trailing \n
    lv_roller_set_options(alarm_hours_roller, hour_opts, LV_ROLLER_MODE_INFINITE);
    lv_obj_add_style(alarm_hours_roller, &style_roller, LV_PART_MAIN);
    lv_obj_add_style(alarm_hours_roller, &style_roller_selected, LV_PART_SELECTED);
    lv_obj_align(alarm_hours_roller, LV_ALIGN_CENTER, -40, 0);
    lv_roller_set_visible_row_count(alarm_hours_roller, 2);
    lv_obj_add_event_cb(alarm_hours_roller, alarm_time_change_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    alarm_minutes_roller = lv_roller_create(screen);
    //lv_obj_set_size(alarm_minutes_roller, 60, 80);
    lv_obj_add_style(alarm_minutes_roller, &style_roller, LV_PART_MAIN);
    lv_obj_add_style(alarm_minutes_roller, &style_roller_selected, LV_PART_SELECTED);
    lv_obj_align_to(alarm_minutes_roller, alarm_hours_roller, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    static char minute_opts[512];
    for(int i = 0; i < 59; i++) {
        sprintf(minute_opts + strlen(minute_opts), "%02d\n", i);
    }
    strcat(minute_opts, "60");
    lv_roller_set_options(alarm_minutes_roller, minute_opts, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(alarm_minutes_roller, 2);
    lv_obj_add_event_cb(alarm_minutes_roller, alarm_time_change_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    static align_cfg_t aligns = {-30, 10, LV_ALIGN_OUT_BOTTOM_MID, LV_TEXT_ALIGN_CENTER};
    static size_cfg_t alarm_label_size = {20, 210};
    alarm_time_label = ui_add_aligned_label(NULL, "Alarm time: 00:00", alarm_minutes_roller, 
        &style_default_medium, &aligns, &alarm_label_size, screen);
    lv_label_set_text_fmt(alarm_time_label, "Alarm time: %02d:%02d", get_int_key_value("ui_alarm_hour", 0), get_int_key_value("ui_alarm_min", 0));

    align_cfg_t btn_align = {20, 10, LV_ALIGN_OUT_BOTTOM_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *alarm_btn = ui_add_button(NULL, "Set", alarm_time_label, &style_default, alarm_btn_event_cb, &btn_align, &btn_size, screen);
    align_cfg_t cancel_btn_align = {10, 0, LV_ALIGN_OUT_RIGHT_MID, LV_TEXT_ALIGN_AUTO};
    size_cfg_t cancel_btn_size = {40, 100};
    lv_obj_t *cancel_alarm_btn = ui_add_button(NULL, "Unset", alarm_btn, &style_default, alarm_cancel_btn_event_cb, &cancel_btn_align, 
        &cancel_btn_size, screen);

    lv_obj_set_scroll_dir(screen, LV_DIR_NONE);
}

void draw_weather_screen()
{
    Serial.println("Drawing alarm screen");
    lv_obj_t *screen = screens[WEATHER_SCREEN];
    align_cfg_t weather_screen_align = {0, 45, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t weather_screen_size = {160, 180};
    weather_screen_label = ui_add_aligned_label(NULL, "No weather data", NULL, &style_default_small, &weather_screen_align, 
        &weather_screen_size, screen);
    align_cfg_t weather_screen_status_align = {20, 0, LV_ALIGN_TOP_RIGHT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t weather_screen_status_size = {160, 40};
    weather_screen_status_label = ui_add_aligned_label(NULL, "", weather_screen_label, &style_weather, &weather_screen_status_align, 
        &weather_screen_status_size, screen);
}

void draw_clock_screen()
{
    Serial.println("Drawing clock screen");
    lv_obj_t *screen = screens[CLOCK_SCREEN];
    // Add clock btn
    align_cfg_t btn_align = {0, 45, LV_ALIGN_TOP_MID, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {70, 180};
    lv_obj_t *time_btn = ui_add_button("time", "00:00:00", NULL, &style_default_large, clock_btn_event_cb, &btn_align, &btn_size, screen);
    time_label = lv_obj_get_child(time_btn, NULL);

    static align_cfg_t aligns = {0, 0, LV_ALIGN_OUT_BOTTOM_MID, LV_TEXT_ALIGN_CENTER};
    static size_cfg_t date_size = {20, 200};
    static size_cfg_t sun_size = {20, 200};
    date_label = ui_add_aligned_label("date", "Date: --", time_btn, &style_default, &aligns, &date_size, screen);

    static int32_t col_dsc[] = {80, 100, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {20, 20, 20, LV_GRID_TEMPLATE_LAST};

    static align_cfg_t grid_aligns = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_LEFT};
    /*Create a container with grid*/
    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_add_style(container, &style_grid, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(container, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(container, row_dsc, 0);
    lv_obj_set_size(container, 180, 70);
    lv_obj_align_to(container, date_label, LV_ALIGN_OUT_BOTTOM_LEFT, 10, 0);
    lv_obj_set_layout(container, LV_LAYOUT_GRID);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *clock_temp_title_label = lv_label_create(container);
    lv_label_set_text(clock_temp_title_label, "Temp:");
    lv_obj_set_grid_cell(clock_temp_title_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 0, 1);
    clock_temp_label = ui_add_aligned_label("clock_temp", "", NULL, &style_default_small, &grid_aligns, NULL, container);
    lv_obj_set_grid_cell(clock_temp_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *clock_wind_title_label = lv_label_create(container);
    lv_label_set_text(clock_wind_title_label, "Wind:");
    lv_obj_set_grid_cell(clock_wind_title_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 1, 1);
    clock_wind_label = ui_add_aligned_label("clock_wind", "", NULL, &style_default_small, &grid_aligns, NULL, container);
    lv_obj_set_grid_cell(clock_wind_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *clock_weather_title_label = lv_label_create(container);
    lv_label_set_text(clock_weather_title_label, "Weather:");
    lv_obj_set_grid_cell(clock_weather_title_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 2, 1);
    current_weather = ui_add_aligned_label(NULL, "", NULL, &style_weather, &grid_aligns, NULL, container);
    lv_obj_set_grid_cell(current_weather, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_STRETCH, 2, 1);

    aligns.x = 5;
    sun_status = ui_add_aligned_label("suntimes", "Rise: --:-- Set: --:--", container, &style_default_small, &aligns, &sun_size, screen);   
}

void switch_to_screen(int screen)
{
    Serial.println("Switching screen");
    lv_scr_load(screens[screen]);
    if (current_screen != screen)   
        current_screen = screen;
}

void screen_swipe_cb(lv_event_t * e) 
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    if (dir == LV_DIR_LEFT) {
        current_screen = (current_screen + 1) % NUM_SCREENS;
    } else if (dir == LV_DIR_RIGHT) {
        current_screen = (current_screen == 0) ? (NUM_SCREENS - 1) : (current_screen - 1);
    }
    switch_to_screen(current_screen);
}

void init_screens()
{
    Serial.println("Init screens");
    screens[CLOCK_SCREEN] = lv_obj_create(NULL);
    screens[STATUS_SCREEN] = lv_obj_create(NULL);
    screens[WEATHER_SCREEN] = lv_obj_create(NULL);
    screens[ALARM_SCREEN] = lv_obj_create(NULL);
    for (int i = 0; i < NUM_SCREENS; i++) {
        lv_obj_set_style_bg_color(screens[i], lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(screens[i], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_add_event_cb(screens[i], screen_swipe_cb, LV_EVENT_GESTURE, NULL);
    }
    draw_clock_screen();
    draw_status_screen();
    draw_alarm_screen();
    draw_weather_screen();
    draw_screen_headers();
}

void draw_status_screen()
{
    lv_obj_t *screen = screens[STATUS_SCREEN];
    lv_obj_t *status_title_label = ui_add_title_label("Status", screen);

    static int32_t col_dsc[] = {100, 140, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {20, 20, 20, 20, 20, 20, LV_GRID_TEMPLATE_LAST};

    static align_cfg_t grid_aligns = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_LEFT};
    /*Create a container with grid*/
    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_add_style(container, &style_grid, LV_PART_MAIN);
    lv_obj_set_style_grid_column_dsc_array(container, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(container, row_dsc, 0);
    lv_obj_set_size(container, 240, 120);
    lv_obj_align(container, LV_ALIGN_TOP_LEFT, 0, 90);
    lv_obj_set_layout(container, LV_LAYOUT_GRID);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *status_ssid_title_label = lv_label_create(container);
    lv_label_set_text(status_ssid_title_label, "SSID:");
    lv_obj_set_grid_cell(status_ssid_title_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 0, 1);
    status_ssid_value_label = lv_label_create(container);
    lv_obj_set_grid_cell(status_ssid_value_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *status_local_ip_title_label = lv_label_create(container);
    lv_label_set_text(status_local_ip_title_label, "Local IP:");
    lv_obj_set_grid_cell(status_local_ip_title_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 1, 1);
    status_local_ip_value_label = lv_label_create(container);
    lv_obj_set_grid_cell(status_local_ip_value_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *status_gateway_ip_title_label = lv_label_create(container);
    lv_label_set_text(status_gateway_ip_title_label, "Gateway IP:");
    lv_obj_set_grid_cell(status_gateway_ip_title_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 2, 1);
    status_gateway_ip_value_label = lv_label_create(container);
    lv_obj_set_grid_cell(status_gateway_ip_value_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_STRETCH, 2, 1);

    lv_obj_t *status_power_title_label = lv_label_create(container);
    lv_label_set_text(status_power_title_label, "Battery:");
    lv_obj_set_grid_cell(status_power_title_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 3, 1);
    status_power_value_label = lv_label_create(container);
    lv_obj_set_grid_cell(status_power_value_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_STRETCH, 3, 1);

    lv_obj_t *status_temp_title_label = lv_label_create(container);
    lv_label_set_text(status_temp_title_label, "Int. Temp.:");
    lv_obj_set_grid_cell(status_temp_title_label, LV_GRID_ALIGN_STRETCH, 0, 1,
                             LV_GRID_ALIGN_STRETCH, 4, 1);
    status_temp_value_label = lv_label_create(container);
    lv_obj_set_grid_cell(status_temp_value_label, LV_GRID_ALIGN_STRETCH, 1, 1,
                             LV_GRID_ALIGN_STRETCH, 4, 1);
}
