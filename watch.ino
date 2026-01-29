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

#define ONE_SECOND 1000
#define FIVE_SECONDS 5000
#define THIRTY_SECONDS 30000
#define ONE_MINUTE 60000
#define THIRTY_MINUTES 1800000
#define TWELVE_HOURS 43200000

const char* ntpServer = "pool.ntp.org";  // European pool


static bool wifi_connected = false;
static unsigned long last_millis = 0, wifi_start_time = 0, last_weather_check = 0, last_wifi_check = 0, last_status_check = 0, last_time_sync = 0;


typedef struct {
    char hours[8], minutes[8];
} alarm_t;



// Fix this. It blocks time updating until wifi connects. Plus it's a mess with the while loop
static void check_wifi()
{
    if (WiFi.status() != WL_CONNECTED) {
        lv_style_set_text_color(&style_wifi, color_red);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    } else {
        wifi_connected = true;
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
        char wifi_cacheBuf[70] = "Wifi not found";
        if (WiFi.status() == WL_CONNECTED) {
            snprintf(wifi_cacheBuf, sizeof(wifi_cacheBuf),
                (const char*)"SSID: %s\nLocal IP: %s\nRouter IP: %s",
                    WiFi.SSID().c_str(), 
                    WiFi.localIP().toString().c_str(), 
                    WiFi.gatewayIP().toString().c_str()
            );
        }
        lv_label_set_text(wifi_status_label, wifi_cacheBuf);
    }
    lv_obj_add_style(wifi_label, &style_wifi, LV_PART_MAIN);
}


void setup()
{
    Serial.begin(115200);
    instance.begin();
    beginLvglHelper(instance);
    init_styles();
    init_screens();
    switch_to_screen(CLOCK_SCREEN);
    check_wifi();
    instance.setBrightness(DEVICE_MAX_BRIGHTNESS_LEVEL);
}

void loop()
{
    lv_timer_handler();

    // simple check for seconds (change to use lv_timer later)
    if (millis() - last_millis >= ONE_SECOND) {
        last_millis = millis();
        update_time();
        if (current_screen == CLOCK_SCREEN)
            update_date();
    }
    if (millis() - last_status_check >= FIVE_SECONDS) {
        last_status_check = millis();
        update_battery_percent();
        refresh_screen_headers();
    }
    if (millis() - last_wifi_check >= ONE_MINUTE) {
        check_wifi();
    }
    if (millis() - last_weather_check >= THIRTY_MINUTES || (!last_weather_check && WiFi.status() == WL_CONNECTED)) {
        last_weather_check = millis();
        if (current_screen == CLOCK_SCREEN) {
            update_weather();
        }        
    }
    delay(20);
}