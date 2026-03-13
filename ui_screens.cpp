#include "widgets/label/lv_label.h"
#include "core/lv_obj_pos.h"
#include "HardwareSerial.h"
#include "ArduinoJson/Array/JsonArray.hpp"
#include "misc/lv_area.h"
#include <lvgl.h>
#include <LilyGoLib.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "nvs.h"
#include "nvs_flash.h"
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
    *status_power_value_label, *status_temp_value_label, *status_vbus_value_label, *status_sysvolt_value_label, 
    *status_mem_value_label, *alarm_time_status_label, *status_ffat_value_label;
lv_obj_t *alarm_time_label, *alarm_hours_roller, *alarm_minutes_roller;
lv_obj_t *popup;
lv_obj_t *weather_screen_label, *weather_screen_status_label;
lv_obj_t *wifi_scan_container;
lv_obj_t *wifi_input_box;
lv_obj_t *settings_utc_textarea, *settings_utc2_textarea, *settings_longitude_textarea, *settings_latitude_textarea;
lv_obj_t *settings_keyboard;
lv_obj_t *settings_mbox;
lv_obj_t *settings_textarea = NULL;

int utc_offset_value, utc2_offset_value;
float longitude_value, latitude_value;

int current_screen = CLOCK_SCREEN;
alarm_cfg_t ui_alarm = {0, 0, false, false, 0};
char wifi_ssid_selected[33];

static void clock_btn_event_cb(lv_event_t *e)
{
    if (monitor.sleeping)
        return;
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
    if (monitor.sleeping)
        wakeup();
    last_event = millis();
}

void alarm_stop()
{
    ui_alarm.end_time = 0;
    ui_alarm.running = false;
    ui_alarm.set = false;
    lv_obj_add_flag(popup, LV_OBJ_FLAG_HIDDEN);
    last_event = millis();
}

void alarm_alert() {
    instance.drv.setWaveform(0, 113);
    instance.drv.run();
}

void update_date() 
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        lv_label_set_text(date_label, get_string_key_value("date", "Date: --").c_str());
        return;
    }
    static lv_obj_t *battery_label;
    char date[20];
    size_t len = strftime(date, sizeof(date), "%a. %Y-%m-%d", &timeinfo);
    String date_str = (String)date;
    put_string_key_value("date", date_str);
    lv_label_set_text(date_label, date_str.c_str());  // "Mon 2026-01-05"
}

void update_time() 
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
        if (current_screen == CLOCK_SCREEN) {
            lv_label_set_text(time_label, get_string_key_value("time", "--:--:--").c_str());
            lv_label_set_text(time_label_2, get_string_key_value("time_2", "--:--:--").c_str());
        } else {
            lv_label_set_text(time_label_2, get_string_key_value("time", "--:--:--").c_str());
        }
        return;
    }
    char time_cacheBuf[10];
    snprintf(time_cacheBuf, sizeof(time_cacheBuf),
        (const char*)"%02d:%02d:%02d",
        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        
    time_t now = mktime(&timeinfo);
    time_t time2 = now + (utc2_offset_value * 3600);
    struct tm time2info;
    localtime_r(&time2, &time2info);
    char time_2_cacheBuf[10];
    snprintf(time_2_cacheBuf, sizeof(time_2_cacheBuf),
        (const char*)"%02d:%02d:%02d",
        time2info.tm_hour, time2info.tm_min, time2info.tm_sec);

    put_string_key_value("time", time_cacheBuf);
    put_string_key_value("time_2", time_2_cacheBuf);
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
        lv_label_set_text(alarm_time_status_label, "Set");
    } else {
        lv_label_set_text(alarm_time_status_label, "Unset");
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
    char longitude_str[16];
    sprintf(longitude_str, "%.4f", longitude_value);
    char latitude_str[16];
    sprintf(latitude_str, "%.4f", latitude_value);
    String url = String("https://api.open-meteo.com/v1/forecast?latitude=")+latitude_str+
        "&longitude="+longitude_str+"&current_weather=true&daily=sunrise,sunset,weather_code,temperature_2m_max,"+
        "temperature_2m_min&timezone=Europe/Vilnius&forecast_days=14";  
    Serial.println("Fetching url:");
    Serial.println(url);
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
            put_string_key_value("suntimes", cacheBuf);
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
            }
            lv_label_set_text(weather_screen_label, String(weatherBuf).c_str());
            lv_label_set_text(weather_screen_status_label, String(weatherStatusBuf).c_str());
        }
    }
    http.end();
}

void ui_update_wifi(int result)
{
    lv_obj_clean(wifi_scan_container);
    lv_obj_t *label = lv_label_create(wifi_scan_container);
    lv_label_set_text_fmt(label, "Result: %d\n", result);
}

static void wifi_item_event_cb(lv_event_t *e)
{
    lv_obj_t *clicked = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *list = lv_obj_get_parent(clicked);

    // Uncheck all siblings
    uint32_t child_cnt = lv_obj_get_child_cnt(list);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(list, i);
        if (child != clicked) {
            lv_obj_clear_state(child, LV_STATE_CHECKED);
        }
    }
    lv_obj_t *ssid_label = lv_obj_get_child(clicked, 0);
    if (ssid_label) {
        lv_obj_t * ssid_label = lv_obj_get_child(clicked, 0);
        const char *text = (const char *)lv_obj_get_user_data(clicked);
        strncpy(wifi_ssid_selected, text, 32);
        wifi_ssid_selected[32] = '\0';  // Force null termination

    }
    lv_obj_add_state(clicked, LV_STATE_CHECKED);
}

// Antenna on twatch is made of cheese, so boost these up a bit. -69 is right next to a router
int rssi_to_bars(int rssi) {
    if (rssi >= -60) return 5;
    if (rssi >= -80) return 4;
    if (rssi >= -90) return 3;
    if (rssi >= -100) return 2;
    return 1;
}

// this crashes sometimes
void ui_print_wifi_scan() {
    lv_obj_clean(wifi_scan_container);
    
    lv_obj_t *list = lv_list_create(wifi_scan_container);
    lv_obj_align(list, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(list, &style_container, LV_PART_MAIN);
    lv_obj_set_width(list, INNER_CONTENT_WIDTH);
    for (size_t i = 0; i < scannedCount; i++) {
        char ssidBuf[64];
        snprintf(ssidBuf, sizeof(ssidBuf), "%s", scannedNetworks[i].ssid);
        if (strcmp(monitor.ssid.c_str(), scannedNetworks[i].ssid) == 0)
            snprintf(ssidBuf, sizeof(ssidBuf), "%s " LV_SYMBOL_WIFI, scannedNetworks[i].ssid);
        if (scannedNetworks[i].encryption != WIFI_AUTH_OPEN) {
            char temp[64];
            snprintf(temp, sizeof(temp), "%s %s", ssidBuf, LV_SYMBOL_WARNING);
            strncpy(ssidBuf, temp, sizeof(ssidBuf));
        }
        char temp[64];
        Serial.printf("Rssi for %s: %d\n", scannedNetworks[i].ssid, scannedNetworks[i].rssi);
        snprintf(temp, sizeof(temp), "%s %d*", ssidBuf, rssi_to_bars(scannedNetworks[i].rssi));
        strncpy(ssidBuf, temp, sizeof(ssidBuf));
        ssidBuf[sizeof(ssidBuf)-1] = '\0';

        lv_obj_t *item = lv_list_add_btn(list, NULL, ssidBuf);
        lv_obj_add_flag(item, LV_OBJ_FLAG_CHECKABLE);
        lv_label_set_long_mode(lv_obj_get_child(item, 0), LV_LABEL_LONG_WRAP);
        lv_obj_add_style(item, &style_container, LV_PART_MAIN);
        lv_obj_add_style(item, &style_default_small, LV_PART_MAIN);
        lv_obj_set_style_bg_color(item, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(item, color_default, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(item, lv_color_black(), LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_add_event_cb(item, wifi_item_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_set_user_data(item, (void*)scannedNetworks[i].ssid);
    }
}

void ui_refresh_sensor_labels()
{
    lv_label_set_text_fmt(status_ssid_value_label, "%s", String(monitor.ssid).c_str());
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

    volts = monitor.usb_voltage / 1000.0f;
    char usb_cacheBuf[20];
    snprintf(usb_cacheBuf, sizeof(usb_cacheBuf),
        "%d.%02dv",
        (int)volts,
        (int)(volts * 100) % 100);
    lv_label_set_text(status_vbus_value_label, usb_cacheBuf);

    volts = monitor.sys_voltage / 1000.0f;
    char sysvolt_cacheBuf[20];
    snprintf(sysvolt_cacheBuf, sizeof(sysvolt_cacheBuf),
        "%d.%02dv",
        (int)volts,
        (int)(volts * 100) % 100);
    lv_label_set_text(status_sysvolt_value_label, sysvolt_cacheBuf);

    char mem_cacheBuf[20];
    snprintf(mem_cacheBuf, sizeof(mem_cacheBuf),
        "%.1f kB", monitor.freemem / 1024.0);
    lv_label_set_text(status_mem_value_label, mem_cacheBuf);
    lv_label_set_text_fmt(status_ffat_value_label, "%.3f/%.2f MB", 
        monitor.disk_used_bytes / (1024.0 * 1024.0),
        monitor.disk_bytes / (1024.0 * 1024.0));
}

void refresh_screen_headers()
{
    char battery_percent_cacheBuf[8];
    snprintf(battery_percent_cacheBuf, sizeof(battery_percent_cacheBuf), "%d%%", monitor.battery_percent);
    lv_label_set_text(battery_label, battery_percent_cacheBuf);
    if (!monitor.wifi_enabled)
        lv_style_set_text_color(&style_wifi, color_grey);
    else {
        if (monitor.wifi_connected)
            lv_style_set_text_color(&style_wifi, color_green);
        else
            lv_style_set_text_color(&style_wifi, color_red);
    }
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
    if (monitor.sleeping)
        return;
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
    if (monitor.sleeping)
        return;
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
    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, alarm_title_label, screen);
    alarm_hours_roller = lv_roller_create(content);
    //lv_obj_set_size(alarm_hours_roller, 60, 80);
    static char hour_opts[64];
    for(int i = 0; i < 23; i++) {
        sprintf(hour_opts + strlen(hour_opts), "%02d\n", i);
    }
    strcat(hour_opts, "23"); // Add seperate to remove trailing \n
    lv_roller_set_options(alarm_hours_roller, hour_opts, LV_ROLLER_MODE_INFINITE);
    lv_obj_add_style(alarm_hours_roller, &style_roller, LV_PART_MAIN);
    lv_obj_add_style(alarm_hours_roller, &style_roller_selected, LV_PART_SELECTED);
    lv_obj_align(alarm_hours_roller, LV_ALIGN_TOP_LEFT, 10, 0);
    lv_roller_set_visible_row_count(alarm_hours_roller, 3);
    lv_obj_add_event_cb(alarm_hours_roller, alarm_time_change_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_roller_set_selected(alarm_hours_roller, get_int_key_value("ui_alarm_hour", 0), LV_ANIM_OFF);

    alarm_minutes_roller = lv_roller_create(content);
    //lv_obj_set_size(alarm_minutes_roller, 60, 80);
    lv_obj_add_style(alarm_minutes_roller, &style_roller, LV_PART_MAIN);
    lv_obj_add_style(alarm_minutes_roller, &style_roller_selected, LV_PART_SELECTED);
    lv_obj_align_to(alarm_minutes_roller, alarm_hours_roller, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    static char minute_opts[128];
    for(int i = 0; i < 59; i++) {
        sprintf(minute_opts + strlen(minute_opts), "%02d\n", i);
    }
    strcat(minute_opts, "60");
    lv_roller_set_options(alarm_minutes_roller, minute_opts, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(alarm_minutes_roller, 3);
    lv_obj_add_event_cb(alarm_minutes_roller, alarm_time_change_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    lv_roller_set_selected(alarm_minutes_roller, get_int_key_value("ui_alarm_min", 0), LV_ANIM_OFF); 

    lv_obj_t *alarm_container = lv_obj_create(content);
    lv_obj_add_style(alarm_container, &style_container, LV_PART_MAIN);
    lv_obj_set_size(alarm_container, 100, 100);
    lv_obj_align_to(alarm_container, alarm_minutes_roller, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    static align_cfg_t aligns = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_CENTER};
    static size_cfg_t alarm_label_size = {20, 80};
    alarm_time_status_label = ui_add_aligned_label(NULL, "Set", NULL, 
        &style_default_small, &aligns, &alarm_label_size, alarm_container);
    aligns.align = LV_ALIGN_OUT_BOTTOM_LEFT;
    alarm_time_label = ui_add_aligned_label(NULL, "00:00", alarm_time_status_label, 
        &style_default_medium, &aligns, &alarm_label_size, alarm_container);
    lv_label_set_text_fmt(alarm_time_label, "%02d:%02d", get_int_key_value("ui_alarm_hour", 0), 
        get_int_key_value("ui_alarm_min", 0));

    lv_obj_t *btn_container = ui_add_button_row(screen);
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *alarm_btn = ui_add_button(NULL, "Set", NULL, &style_default_small, alarm_btn_event_cb, 
        &btn_align, &btn_size, btn_container);
    align_cfg_t cancel_btn_align = {0, 0, LV_ALIGN_TOP_RIGHT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t cancel_btn_size = {40, 100};
    lv_obj_t *cancel_alarm_btn = ui_add_button(NULL, "Unset", NULL, &style_default_small, 
        alarm_cancel_btn_event_cb, &cancel_btn_align, &cancel_btn_size, btn_container);

    lv_obj_set_scroll_dir(screen, LV_DIR_NONE);
}

void draw_weather_screen()
{
    Serial.println("Drawing alarm screen");
    lv_obj_t *screen = screens[WEATHER_SCREEN];
    lv_obj_t *weather_title_label = ui_add_title_label("Weather", screen);
    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT, weather_title_label, screen);
    align_cfg_t weather_screen_align = {0, 0, LV_ALIGN_OUT_BOTTOM_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t weather_screen_size = {140, 180};
    weather_screen_label = ui_add_aligned_label(NULL, "No weather data", NULL, &style_default_small, &weather_screen_align, 
        &weather_screen_size, content);
    align_cfg_t weather_screen_status_align = {20, 0, LV_ALIGN_TOP_RIGHT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t weather_screen_status_size = {160, 40};
    weather_screen_status_label = ui_add_aligned_label(NULL, "", weather_screen_label, &style_weather, &weather_screen_status_align, 
        &weather_screen_status_size, content);
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
    last_event = millis();
}

void screen_swipe_cb(lv_event_t * e) 
{
    if (monitor.sleeping)
        return;
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
    screens[WIFI_SCREEN] = lv_obj_create(NULL);
    for (int i = 0; i < NUM_SCREENS; i++) {
        lv_obj_set_style_bg_color(screens[i], lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(screens[i], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_add_event_cb(screens[i], screen_swipe_cb, LV_EVENT_GESTURE, NULL);
    }
    draw_clock_screen();
    draw_status_screen();
    draw_alarm_screen();
    draw_weather_screen();
    draw_wifi_screen();
    draw_screen_headers();

    // Secondary screens (these are not shown in the normal screen loop)
    secondary_screens[SETTINGS_SCREEN] = lv_obj_create(NULL);
    secondary_screens[CACHE_SCREEN] = lv_obj_create(NULL);
    for (int i = 0; i < NUM_SECONDARY_SCREENS; i++) {
        lv_obj_set_style_bg_color(secondary_screens[i], lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(secondary_screens[i], LV_OPA_COVER, LV_PART_MAIN);
    }
    draw_settings_screen();
    draw_cache_screen();
}

void back_button_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        switch_to_screen(current_screen);
    }
}

void wifi_switch_event_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target_obj(e);

    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        toggle_wifi();
    }       
}

void settings_kb_event_cb(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target_obj(e);

    if(code == LV_EVENT_CANCEL) {   
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_delete(settings_mbox);
        settings_mbox = NULL;
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);                              
        return;
    }

    if(code == LV_EVENT_READY) { 
        const char *settings_value = lv_textarea_get_text(settings_textarea);
        lv_obj_t *original_textbox = (lv_obj_t *)lv_obj_get_user_data(settings_mbox);
        msgbox_data_t *mbox_data = (msgbox_data_t *)lv_obj_get_user_data(original_textbox);
        const char *cache_key = (const char*)mbox_data->cache_key;

        Serial.println(cache_key);
        Serial.println(settings_value);
        Serial.println(lv_textarea_get_text(original_textbox));

        if (mbox_data->default_float_ptr != NULL) {
            float settings_float = strtof(settings_value, NULL);
            *mbox_data->default_float_ptr = settings_float;
            put_float_key_value(cache_key, settings_float);

        } else if (mbox_data->default_int_ptr != NULL) {
            int settings_int = strtol(settings_value, NULL, 10);
            *mbox_data->default_int_ptr = settings_int;
            put_int_key_value(cache_key, settings_int);

            Serial.printf("Writing %d to %s\n", settings_int, cache_key);
            Serial.printf("Contents of cache: %d\n", get_int_key_value(cache_key, -99));
        }
        lv_textarea_set_text(original_textbox, settings_value);
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_delete(settings_mbox);
        settings_mbox = NULL;
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);                             
        return;
    }
}


static void settings_input_click_cb(lv_event_t * e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target_input = lv_event_get_target_obj(e);

    if (code == LV_EVENT_CLICKED) {
        msgbox_data_t *mbox_data = (msgbox_data_t *)lv_obj_get_user_data(target_input);
        const char *title_text = (const char*)mbox_data->title;
        const char *default_text = lv_textarea_get_text(target_input);
        settings_mbox = ui_show_input_box(title_text, default_text, lv_layer_top(), &settings_textarea);

        lv_obj_set_user_data(settings_mbox, target_input);
        lv_keyboard_set_textarea(settings_keyboard, settings_textarea);
        lv_obj_clear_flag(settings_keyboard, LV_OBJ_FLAG_HIDDEN);
    } 
}

void draw_cache_screen()
{
    lv_obj_t *screen = secondary_screens[CACHE_SCREEN];
    lv_obj_t *cache_title_label = ui_add_title_label("Cache/Storage", screen);

    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, cache_title_label, screen);

    nvs_stats_t stats;
    nvs_get_stats(NULL, &stats);

    lv_obj_t *used_entries = lv_label_create(content);
    lv_label_set_text_fmt(used_entries, "Used: %d", stats.used_entries);
    lv_obj_t *free_entries = lv_label_create(content);
    lv_label_set_text_fmt(free_entries, "Free: %d", stats.free_entries);
    lv_obj_align_to(free_entries, used_entries, LV_ALIGN_BOTTOM_LEFT, 0, 20);  
    lv_obj_t *total_entries = lv_label_create(content);
    lv_label_set_text_fmt(total_entries, "Total: %d", stats.total_entries);    
    lv_obj_align_to(total_entries, free_entries, LV_ALIGN_BOTTOM_LEFT, 0, 20);   
    lv_obj_t *namespaces = lv_label_create(content);
    lv_label_set_text_fmt(namespaces, "Namespaces: %d", stats.namespace_count);   
    lv_obj_align_to(namespaces, total_entries, LV_ALIGN_BOTTOM_LEFT, 0, 20); 

    lv_obj_t *btn_container = ui_add_button_row(screen);
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *settings_btn = ui_add_button(NULL, "Back", NULL, &style_default_small, back_button_cb, 
        &btn_align, &btn_size, btn_container);
}

void draw_settings_screen()
{
    lv_obj_t *screen = secondary_screens[SETTINGS_SCREEN];
    lv_obj_t *settings_title_label = ui_add_title_label("Settings", screen);   
    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, settings_title_label, screen);
    
    static int32_t col_dsc[] = {140, 80, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {30, 30, 30, 30, 30, 30, 30, LV_GRID_TEMPLATE_LAST};
    const char *utc_offset = "UTC Offset", *utc2_offset = "UTC2 Offset", *longitude = "Longitude", *latitude = "Latitude";

    static grid_row_t rows[] = {
        {"Toggle Wifi", NULL},
        {"Toggle BT", NULL},
        {"Toggle GPS", NULL},
        {utc_offset, NULL},
        {utc2_offset, NULL},
        {longitude, NULL},
        {latitude, NULL}
    };
    lv_obj_t *grid = ui_create_grid(col_dsc, row_dsc, rows, 7, content);

    lv_obj_t *wifi_switch = lv_switch_create(grid);
    lv_obj_add_event_cb(wifi_switch, wifi_switch_event_cb, LV_EVENT_ALL, NULL);
    if (monitor.wifi_enabled)
        lv_obj_add_state(wifi_switch, LV_STATE_CHECKED);
    lv_obj_set_grid_cell(wifi_switch, LV_GRID_ALIGN_STRETCH, 1, 1, 
        LV_GRID_ALIGN_STRETCH, 0, 1);  

    settings_utc_textarea = lv_textarea_create(grid);   
    lv_obj_add_style(settings_utc_textarea, &style_container, LV_PART_MAIN);                              
    lv_textarea_set_one_line(settings_utc_textarea, true);                               
    lv_obj_set_width(settings_utc_textarea, lv_pct(100));  
    lv_obj_set_grid_cell(settings_utc_textarea, LV_GRID_ALIGN_STRETCH, 1, 1, 
        LV_GRID_ALIGN_STRETCH, 3, 1);  
    lv_textarea_set_text(settings_utc_textarea, String(get_int_key_value("utc_offset", DEFAULT_UTC_OFFSET)).c_str());
    lv_obj_add_event_cb(settings_utc_textarea, settings_input_click_cb, LV_EVENT_CLICKED, NULL);
    msgbox_data_t *utc_data = (msgbox_data_t *)malloc(sizeof(msgbox_data_t));
    utc_data->parent_textarea = settings_utc_textarea;
    utc_data->title = utc_offset;
    utc_data->cache_key = "utc_offset";
    utc_data->default_float_ptr = NULL;
    utc_data->default_int_ptr = &utc_offset_value;
    lv_obj_set_user_data(settings_utc_textarea, utc_data);

    settings_utc2_textarea = lv_textarea_create(grid);   
    lv_obj_add_style(settings_utc2_textarea, &style_container, LV_PART_MAIN);                              
    lv_textarea_set_one_line(settings_utc2_textarea, true);                               
    lv_obj_set_width(settings_utc2_textarea, lv_pct(100));  
    lv_obj_set_grid_cell(settings_utc2_textarea, LV_GRID_ALIGN_STRETCH, 1, 1, 
        LV_GRID_ALIGN_STRETCH, 4, 1);  
    lv_textarea_set_text(settings_utc2_textarea, String(get_int_key_value("utc2_offset", DEFAULT_UTC2_OFFSET)).c_str());
    lv_obj_add_event_cb(settings_utc2_textarea, settings_input_click_cb, LV_EVENT_CLICKED, NULL);
    msgbox_data_t *utc2_data = (msgbox_data_t *)malloc(sizeof(msgbox_data_t));
    utc2_data->parent_textarea = settings_utc2_textarea;
    utc2_data->title = utc2_offset;
    utc2_data->cache_key = "utc2_offset";
    utc2_data->default_float_ptr = NULL;
    utc2_data->default_int_ptr = &utc2_offset_value;
    lv_obj_set_user_data(settings_utc2_textarea, utc2_data);

    settings_longitude_textarea = lv_textarea_create(grid);   
    lv_obj_add_style(settings_longitude_textarea, &style_container, LV_PART_MAIN);                              
    lv_textarea_set_one_line(settings_longitude_textarea, true);                               
    lv_obj_set_width(settings_longitude_textarea, lv_pct(100));  
    lv_obj_set_grid_cell(settings_longitude_textarea, LV_GRID_ALIGN_STRETCH, 1, 1, 
        LV_GRID_ALIGN_STRETCH, 5, 1);
    float longitude_value = get_float_key_value("longitude", DEFAULT_LONGITUDE_VALUE);
    char longitude_str[16];
    sprintf(longitude_str, "%.4f", longitude_value);
    lv_textarea_set_text(settings_longitude_textarea, longitude_str);
    lv_obj_add_event_cb(settings_longitude_textarea, settings_input_click_cb, LV_EVENT_CLICKED, NULL);
    msgbox_data_t *longitude_data = (msgbox_data_t *)malloc(sizeof(msgbox_data_t));
    longitude_data->parent_textarea = settings_longitude_textarea;
    longitude_data->title = longitude;
    longitude_data->cache_key = "longitude";
    longitude_data->default_int_ptr = NULL;
    longitude_data->default_float_ptr = &longitude_value;
    lv_obj_set_user_data(settings_longitude_textarea, longitude_data);

    settings_latitude_textarea = lv_textarea_create(grid);   
    lv_obj_add_style(settings_latitude_textarea , &style_container, LV_PART_MAIN);                              
    lv_textarea_set_one_line(settings_latitude_textarea, true);                               
    lv_obj_set_width(settings_latitude_textarea, lv_pct(100));  
    lv_obj_set_grid_cell(settings_latitude_textarea, LV_GRID_ALIGN_STRETCH, 1, 1, 
        LV_GRID_ALIGN_STRETCH, 6, 1);
    float latitude_value = get_float_key_value("latitude", DEFAULT_LATITUDE_VALUE);
    char latitude_str[16];
    sprintf(latitude_str, "%.4f", latitude_value);
    lv_textarea_set_text(settings_latitude_textarea, latitude_str);  
    lv_obj_add_event_cb(settings_latitude_textarea, settings_input_click_cb, LV_EVENT_CLICKED, NULL);
    msgbox_data_t *latitude_data = (msgbox_data_t *)malloc(sizeof(msgbox_data_t));
    latitude_data->parent_textarea = settings_latitude_textarea;
    latitude_data->title = latitude;
    latitude_data->cache_key = "latitude";
    latitude_data->default_int_ptr = NULL;
    latitude_data->default_float_ptr = &latitude_value;
    lv_obj_set_user_data(settings_latitude_textarea, latitude_data);

    lv_obj_t *btn_container = ui_add_button_row(screen);
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *settings_btn = ui_add_button(NULL, "Back", NULL, &style_default_small, back_button_cb, 
        &btn_align, &btn_size, btn_container);

    // Need to draw after the button bar, or it covers
    settings_keyboard = lv_keyboard_create(screen);
    lv_obj_add_style(settings_keyboard, &style_keyboard, LV_PART_MAIN);
    lv_obj_add_style(settings_keyboard, &style_keyboard, LV_PART_ITEMS);
    lv_obj_add_style(settings_keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_add_style(settings_keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_add_style(settings_keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_FOCUSED);
    lv_obj_add_style(settings_keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_DISABLED);
    lv_keyboard_set_mode(settings_keyboard, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_add_flag(settings_keyboard, LV_OBJ_FLAG_HIDDEN); 
    lv_obj_add_event_cb(settings_keyboard, settings_kb_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(settings_keyboard, settings_kb_event_cb, LV_EVENT_CANCEL, NULL);
}

void cache_button_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_scr_load(secondary_screens[CACHE_SCREEN]);
    }
}

void settings_button_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_scr_load(secondary_screens[SETTINGS_SCREEN]);
    }
}

void draw_status_screen()
{
    lv_obj_t *screen = screens[STATUS_SCREEN];
    lv_obj_t *status_title_label = ui_add_title_label("Status", screen);
    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, status_title_label, screen);
    static int32_t col_dsc[] = {100, 140, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, LV_GRID_TEMPLATE_LAST};

    static grid_row_t rows[] = {
        {"SSID:", &status_ssid_value_label},
        {"Local IP:", &status_local_ip_value_label},
        {"Gateway IP:", &status_gateway_ip_value_label},
        {"Battery:", &status_power_value_label},
        {"Int. Temp.:", &status_temp_value_label},
        {"Vbus Volt:", &status_vbus_value_label},
        {"Sys Volt:", &status_sysvolt_value_label},
        {"Free mem.:", &status_mem_value_label},
        {"Storage:", &status_ffat_value_label}
    };
    lv_obj_t *grid = ui_create_grid(col_dsc, row_dsc, rows, 9, content);

    lv_obj_t *btn_container = ui_add_button_row(screen);
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *settings_btn = ui_add_button(NULL, LV_SYMBOL_SETTINGS, NULL, &style_default_small, settings_button_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_RIGHT;
    lv_obj_t *cache_btn = ui_add_button(NULL, "Cache", NULL, &style_default_small, cache_button_cb, 
        &btn_align, &btn_size, btn_container);
}

static lv_obj_t *wifi_password_text;

void kb_event_cb(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target_obj(e);

    if(code == LV_EVENT_CANCEL) {   
        lv_obj_delete(kb);                                    
        lv_obj_delete(wifi_input_box);
        return;
    }

    /* From msgbox buttons: VALUE_CHANGED when OK/Cancel clicked */
    if(code == LV_EVENT_READY) { 
        const char *wifi_password = lv_textarea_get_text(wifi_password_text);     
        Serial.printf("Pass: %s\n", wifi_password);  
        Serial.printf("SSID: %s\n", wifi_ssid_selected);
        WiFi.begin(wifi_ssid_selected, wifi_password);
        save_wifi_to_file(wifi_ssid_selected, wifi_password);            
        lv_obj_delete(kb);                                    
        lv_obj_delete(wifi_input_box);
        return;
    }
}

lv_obj_t *show_input_box()
{
    /* Modal message box */
    lv_obj_t *mbox = lv_msgbox_create(screens[current_screen]);                                            
    //lv_msgbox_add_text(mbox, "Password:");                     
    lv_obj_set_width(mbox, 220);
    /* Content area of msgbox */
    lv_msgbox_add_text(mbox, wifi_ssid_selected);
    lv_obj_t *content = lv_msgbox_get_content(mbox);                  
    lv_obj_add_style(mbox, &style_container, LV_PART_MAIN);
    lv_obj_add_style(mbox, &style_default_small, LV_PART_MAIN);
    /* Text area inside msgbox */
    wifi_password_text = lv_textarea_create(content);   
    lv_obj_add_style(wifi_password_text, &style_container, LV_PART_MAIN);                              
    lv_textarea_set_one_line(wifi_password_text, true);                               
    lv_obj_set_width(wifi_password_text, lv_pct(100));         

    lv_obj_align(mbox, LV_ALIGN_TOP_MID, 0, 40);
    return mbox;
}

void wifi_connect_button_cb(lv_event_t *e) 
{ 
    if (monitor.sleeping)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *keyboard = lv_keyboard_create(screens[current_screen]);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_MAIN);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_PRESSED);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_FOCUSED);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_DISABLED);
        /* Send characters into our textarea */
        wifi_input_box = show_input_box();
        const char *saved_password = get_wifi_password_for_ssid(wifi_ssid_selected);
        if (saved_password)
            lv_textarea_set_text(wifi_password_text, saved_password);
        lv_keyboard_set_textarea(keyboard, wifi_password_text);
        lv_obj_add_event_cb(keyboard, kb_event_cb, LV_EVENT_ALL, NULL);
        Serial.println("Connect button clicked");
    }
}

void wifi_scan_btn_cb(lv_event_t *e)
{
    if (monitor.sleeping)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
       start_wifi_scan();
    }
}

void draw_wifi_screen()
{
    lv_obj_t *screen = screens[WIFI_SCREEN];
    lv_obj_t *title = ui_add_title_label("Wifi", screen);
    wifi_scan_container = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, title, screen);
    Serial.println("Drawing wifi screen");
    
    lv_obj_t *btn_container = ui_add_button_row(screen);
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *settings_btn = ui_add_button(NULL, LV_SYMBOL_SETTINGS, NULL, &style_default_small, wifi_scan_btn_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_MID;
    lv_obj_t *scan_btn = ui_add_button(NULL, LV_SYMBOL_REFRESH, NULL, &style_default_small, wifi_scan_btn_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_RIGHT;
    lv_obj_t *connect_btn = ui_add_button(NULL, "Connect", NULL, &style_default_small, wifi_connect_button_cb, 
        &btn_align, &btn_size, btn_container);
}
