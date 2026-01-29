#include <lvgl.h>
#include <LilyGoLib.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "cache.h"
#include "ui_base.h"
#include "ui_screens.h"
#include "config.h"

lv_obj_t *time_label, *time_label_2;
lv_obj_t *date_label, *outside_weather, *sun_status;
lv_obj_t *status_label;
lv_obj_t *wifi_label, *battery_label, *charge_label, *bluetooth_label, *gps_label;
lv_obj_t *wifi_status_label, *power_status_label;
lv_obj_t *alarm_time_label, *alarm_hours_roller, *alarm_minutes_roller;

int current_screen = CLOCK_SCREEN;

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
}

void update_battery_percent()
{
    lv_label_set_text_fmt(battery_label, "%d%%", get_battery_percent_remaining());
    if (instance.pmu.isCharging()) {
        lv_label_set_text(charge_label, LV_SYMBOL_CHARGE);
    } else {
        lv_label_set_text(charge_label, "");
    }
    char battery_cacheBuf[20] = "Battery status";
    snprintf(battery_cacheBuf, sizeof(battery_cacheBuf),
        (const char*)"Batt: %d%% %.2fv",
        get_battery_percent_remaining(), (instance.pmu.getBattVoltage() / 1000.0f));
    lv_label_set_text(power_status_label, battery_cacheBuf);
}

String wind_dir_to_text(float deg) {
  // normalize
  if (deg < 0) deg += 360;
  if (deg >= 360) deg -= 360;

  static const char* dirs[] = {"N","NE","E","SE","S","SW","W","NW","N"};
  int index = (int)round(deg / 45.0);   // 360 / 8 = 45°
  return String(dirs[index]);
}

String get_weather_icon(int code) 
{
    if (code == 0)
        return "sunny";
    if (code >= 1 && code <= 19)
        return "cloudy";
    if ((code >= 20 && code <= 29) || (code >= 50 && code <= 69))
        return "some rain";
    if (code >= 40 && code <= 49)
        return "foggy";
    if ((code >= 30 && code <= 39) || (code >= 70 && code <= 78))
        return "snow";
    if (code == 79)
        return "ice";
    return "rain";
}

void update_weather()
{
    String url = String("https://api.open-meteo.com/v1/forecast?latitude=")+LATITUDE+
        "&longitude="+LONGITUDE+"&current_weather=true&daily=sunrise,sunset&timezone=Europe/Vilnius";  
    HTTPClient http;
    http.begin(url);
    int code = http.GET();
    if (code == 200) {
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
            String weatherIcon = get_weather_icon(weatherCode);
            int tPos = sunrise.indexOf('T');
            sunrise = sunrise.substring(tPos+1, tPos+6);
            sunset = sunset.substring(tPos+1, tPos+6);
            Serial.println("Temp: " + String(temp) + "°C");
            char cacheBuf[128];
            snprintf(cacheBuf, sizeof(cacheBuf),
                (const char*)"Temp: %.1f°C\nWind: %.1fkm/h %s\nWeather: %s", temp, wind, windDir.c_str(), weatherIcon.c_str());
            putStringKV("weather", cacheBuf);
            lv_label_set_text(outside_weather, cacheBuf);    
            snprintf(cacheBuf, sizeof(cacheBuf),
                "Rise: %s Set: %s", sunrise, sunset);
            putStringKV("suntimes", cacheBuf);
            lv_label_set_text(sun_status, cacheBuf);
        }
    }
    http.end();
}

void refresh_screen_headers()
{
    if (WiFi.status() == WL_CONNECTED)
        lv_style_set_text_color(&style_wifi, color_green);
    else
        lv_style_set_text_color(&style_wifi, color_red);
    if (instance.pmu.isCharging()) {
        lv_obj_clear_flag(charge_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(wifi_label, charge_label, LV_ALIGN_TOP_RIGHT, -20, 0);
    } else {
        lv_obj_add_flag(charge_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align_to(wifi_label, battery_label, LV_ALIGN_TOP_RIGHT, -40, 0);
    }
    // These need to be realigned if wifi label changes position
    lv_obj_align_to(bluetooth_label, wifi_label, LV_ALIGN_TOP_RIGHT, -30, 0);
    lv_obj_align_to(gps_label, bluetooth_label, LV_ALIGN_TOP_RIGHT, -20, 0);
}

void draw_screen_headers()
{
    lv_obj_t *header = lv_obj_create(lv_layer_top());
    lv_obj_set_size(header, LV_HOR_RES, 40);
    lv_obj_set_style_bg_color(header, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);

    static align_cfg_t aligns = {0, 0, LV_ALIGN_TOP_RIGHT, LV_TEXT_ALIGN_AUTO};

    battery_label = ui_add_aligned_label(NULL, "100%", NULL, &style_default_medium, &aligns, NULL, header);

    aligns.x = -45;
    charge_label = ui_add_aligned_label(NULL, LV_SYMBOL_CHARGE, battery_label, &style_charge, &aligns, NULL, header);

    aligns.x = -40;
    wifi_label = ui_add_aligned_label(NULL, LV_SYMBOL_WIFI, battery_label, &style_wifi, &aligns, NULL, header);

    aligns.x = -20;
    bluetooth_label = ui_add_aligned_label(NULL, LV_SYMBOL_BLUETOOTH, wifi_label, &style_inactive, &aligns, NULL, header);
    gps_label = ui_add_aligned_label(NULL, LV_SYMBOL_GPS, bluetooth_label, &style_inactive, &aligns, NULL, header);
    
    aligns = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    time_label_2 = ui_add_aligned_label("time_2", "--:--:--", NULL, &style_default_medium, &aligns, NULL, header);
    refresh_screen_headers(); 
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
        snprintf(timeBuf, sizeof(timeBuf), "%s:%s", hoursBuf, minutesBuf);
        lv_label_set_text(alarm_time_label, timeBuf);
    }
}

void alarm_time_change_event_handler(lv_event_t *e) 
{
   return;
}

void draw_alarm_screen()
{
    lv_obj_t *screen = screens[ALARM_SCREEN];

    alarm_hours_roller = lv_roller_create(screen);
    lv_obj_set_size(alarm_hours_roller, 60, 100);
    static char hour_opts[200];
    for(int i = 0; i < 24; i++) {
        sprintf(hour_opts + strlen(hour_opts), "%02d\n", i);
    }
    lv_roller_set_options(alarm_hours_roller, hour_opts, LV_ROLLER_MODE_INFINITE);
    lv_obj_add_style(alarm_hours_roller, &style_roller, LV_PART_MAIN);
    lv_obj_add_style(alarm_hours_roller, &style_roller_selected, LV_PART_SELECTED);
    lv_obj_align(alarm_hours_roller, LV_ALIGN_CENTER, -40, 0);
    lv_roller_set_visible_row_count(alarm_hours_roller, 3);
    lv_obj_add_event_cb(alarm_hours_roller, alarm_time_change_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    alarm_minutes_roller = lv_roller_create(screen);
    lv_obj_set_size(alarm_minutes_roller, 60, 100);
    lv_obj_add_style(alarm_minutes_roller, &style_roller, LV_PART_MAIN);
    lv_obj_add_style(alarm_minutes_roller, &style_roller_selected, LV_PART_SELECTED);
    lv_obj_align_to(alarm_minutes_roller, alarm_hours_roller, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    static char minute_opts[512];
    for(int i = 0; i < 60; i++) {
        sprintf(minute_opts + strlen(minute_opts), "%02d\n", i);
    }
    lv_roller_set_options(alarm_minutes_roller, minute_opts, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(alarm_minutes_roller, 3);
    lv_obj_add_event_cb(alarm_minutes_roller, alarm_time_change_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    align_cfg_t btn_align = {0, 120, LV_ALIGN_TOP_MID, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *alarm_btn = ui_add_button(NULL, "Set", alarm_hours_roller, &style_default, alarm_btn_event_cb, &btn_align, &btn_size, screen);

    static align_cfg_t aligns = {50, 5, LV_ALIGN_BOTTOM_LEFT, LV_TEXT_ALIGN_CENTER};
    static size_cfg_t alarm_label_size = {20, 100};
    alarm_time_label = ui_add_aligned_label("alarm_time_label", "Alarm time", alarm_btn, 
        &style_default_medium, &aligns, &alarm_label_size, screen);
    lv_obj_set_scroll_dir(screen, LV_DIR_NONE);
}

void draw_clock_screen()
{
    lv_obj_t *screen = screens[CLOCK_SCREEN];
    // Add clock btn
    align_cfg_t btn_align = {0, 45, LV_ALIGN_TOP_MID, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {70, 180};
    lv_obj_t *time_btn = ui_add_button("time", "00:00:00", NULL, &style_default_large, clock_btn_event_cb, &btn_align, &btn_size, screen);
    time_label = lv_obj_get_child(time_btn, NULL);

    static align_cfg_t aligns = {0, 5, LV_ALIGN_OUT_BOTTOM_MID, LV_TEXT_ALIGN_CENTER};
    static size_cfg_t date_size = {20, 200};
    static size_cfg_t weather_size = {60, 200};
    static size_cfg_t sun_size = {20, 200};
    date_label = ui_add_aligned_label("date", "Date: --", time_btn, &style_default, &aligns, &date_size, screen);
    outside_weather = ui_add_aligned_label("weather", "Fetching weather..", date_label, &style_default_medium, &aligns, &weather_size, screen);
    sun_status = ui_add_aligned_label("suntimes", "Rise: --:-- Set: --:--", outside_weather, &style_default_small, &aligns, &sun_size, screen);   
}

void switch_to_screen(int screen)
{
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
    screens[CLOCK_SCREEN] = lv_obj_create(NULL);
    screens[STATUS_SCREEN] = lv_obj_create(NULL);
    screens[ALARM_SCREEN] = lv_obj_create(NULL);
    for (int i = 0; i < NUM_SCREENS; i++) {
        lv_obj_set_style_bg_color(screens[i], lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(screens[i], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_add_event_cb(screens[i], screen_swipe_cb, LV_EVENT_GESTURE, NULL);
    }
    draw_screen_headers();
    draw_clock_screen();
    draw_status_screen();
    draw_alarm_screen();
}


int get_battery_percent_remaining()
{
    float volt = instance.pmu.getBattVoltage() / 1000.0f;
    int percent = constrain((int)((volt - 3.0f) / (4.2f - 3.0f) * 100.0f), 0, 100);
    return percent;
}

void draw_status_screen()
{
    lv_obj_t *screen = screens[STATUS_SCREEN];
    // Add btn
    align_cfg_t status_label_align = {0, 45, LV_ALIGN_TOP_MID, LV_TEXT_ALIGN_AUTO};
    size_cfg_t status_label_size = {70, 180};
    lv_obj_t *status_label = ui_add_aligned_label(NULL, "Status", NULL, &style_default_large, &status_label_align, &status_label_size, screen);
    
    static align_cfg_t aligns = {-20, 5, LV_ALIGN_OUT_BOTTOM_LEFT, LV_TEXT_ALIGN_LEFT};
    static size_cfg_t wifi_size = {80, 200};
    static size_cfg_t battery_size = {20, 200};
    char wifi_cacheBuf[70] = "Wifi not connected";
    wifi_status_label = ui_add_aligned_label(NULL, wifi_cacheBuf, status_label, &style_default_small, &aligns, &wifi_size, screen);

    char battery_cacheBuf[20] = "Battery status";
    aligns.x = 0;
    aligns.y = 0;
    power_status_label = ui_add_aligned_label(NULL, battery_cacheBuf, wifi_status_label, &style_default, &aligns, &battery_size, screen);
    screens[1] = screen;
}
