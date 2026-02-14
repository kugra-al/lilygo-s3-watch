/**
 * @file      watch.ino
 * @author    Kugra-al
 * @license   MIT
 * @copyright Copyright (c) 2026  Kugra-al
 * @date      2026-01-04
 *
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <lvgl.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "time.h"
#include "config.h"
#include <Preferences.h>
#include "ui_base.h"
#include "cache.h"
#include "ui_screens.h"
#include "watch.h"
#include "hw_monitor.h"

const char* ntpServer = "pool.ntp.org";  // European pool
int last_button_click = 0;
static unsigned long last_millis = 0, wifi_start_time = 0, last_weather_check = 0, 
    last_wifi_check = 0, last_status_check = 0, last_time_sync = 0;


typedef struct {
    char hours[8], minutes[8];
} alarm_t;

static void check_wifi()
{
    Serial.println("Wifi check");
    if (!monitor.wifi_connected) {
        lv_style_set_text_color(&style_wifi, color_red);
        Serial.println("Attempting wifi connect");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    } else {
        lv_style_set_text_color(&style_wifi, color_green);
        Serial.print("WiFi connected! IP: ");
        Serial.println(WiFi.localIP());

        if (!last_time_sync) {
            configTime(GMTOFFSET_SEC, DAYLIGHTOFFSET_SEC, ntpServer);
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                last_time_sync = 1;
                Serial.println("NTP time synced!");
                Serial.printf("Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            } 
        }
    }
    lv_obj_add_style(wifi_label, &style_wifi, LV_PART_MAIN);
}

void setup()
{
    Serial.begin(115200);
    instance.begin();
    beginLvglHelper(instance);
    instance.setBrightness(DEVICE_MAX_BRIGHTNESS_LEVEL);
    instance.onEvent([](DeviceEvent_t event, void *params, void * user_data) {
        if (instance.getPMUEventType(params) == PMU_EVENT_KEY_CLICKED) {
            last_event = millis();
            if (monitor.sleeping)
                wakeup();
            Serial.println("Power button pressed");
            if (millis() - last_button_click <= ONE_SECOND) {
                Serial.println("Double click detected");
                fake_sleep();
            }
            last_button_click = millis();
        } else if (instance.getPMUEventType(params) == PMU_EVENT_KEY_LONG_PRESSED) {
            Serial.println("Power button long pressed");
        }
    }, POWER_EVENT, NULL);

    mount_file_system();
    hw_update_monitor();
    init_styles();
    init_screens();
    switch_to_screen(CLOCK_SCREEN);
    check_wifi();
    ui_alarm.hour = get_int_key_value("ui_alarm_hour", 0);
    ui_alarm.minute = get_int_key_value("ui_alarm_min", 0);
    ui_alarm.set = get_bool_key_value("ui_alarm_set", false);
}

void loop()
{
    lv_timer_handler();
    instance.loop();
    if (!monitor.sleeping) {
        int current_millis = millis();
        // simple check for seconds (change to use lv_timer later)
        if (current_millis - last_millis >= ONE_SECOND) {
            last_millis = current_millis;
            update_time();
            if (current_screen == CLOCK_SCREEN) // Move to check if there's no valid date, or midnight
                update_date();
        }
        if (current_millis - last_status_check >= FIVE_SECONDS) {
            last_status_check = current_millis;
            hw_update_monitor();
            refresh_screen_headers();
            ui_refresh_sensor_labels();
        }
        if (current_millis - last_wifi_check >= ONE_MINUTE) {
            last_wifi_check = current_millis;
            check_wifi();
        }
        if (last_event && current_millis - last_event >= TWO_MINUTES) {
            // Fake sleep because wakeup from power key doesn't work correctly  
            fake_sleep();
        }

        if (monitor.wifi_connected && (current_millis - last_weather_check >= THIRTY_MINUTES || !last_weather_check)) {
            last_weather_check = current_millis;
            update_weather();      
        }
    }
    delay(250);
}