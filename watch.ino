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
#include "nvs.h"
#include "nvs_flash.h"
#include "time.h"
#include <Preferences.h>
#include "ui_screens.h"

#define LEFT_BTN_PIN 0
volatile bool left_btn_pressed = false;
bool torch_active = false;
const char* ntpServer = "pool.ntp.org";  // European pool
int last_button_click = 0;
static unsigned long last_millis = 0, wifi_start_time = 0, last_weather_check = 0, 
    last_wifi_check = 0, last_status_check = 0, last_time_sync = 0;
bool saved_defined_network = false, wifi_scanning = false;
int wifi_connection_attempts = 0;

typedef struct {
    char hours[8], minutes[8];
} alarm_t;

wifi_t *scannedNetworks = nullptr;
size_t  scannedCount    = 0;

void IRAM_ATTR handleLeftButtonPress() {
    left_btn_pressed = true;
}

void toggle_wifi()
{
    if (monitor.wifi_enabled) {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        wifi_enabled = false;
        wifi_disable_ap();
    } else {
        start_wifi_scan();
        wifi_enabled = true;
    }
}

bool ssid_exists(const char *ssid) {
    for (size_t i = 0; i < scannedCount; ++i) {
        if (strcmp(scannedNetworks[i].ssid, ssid) == 0) {
            return true;
        }
    }
    return false;
}

static void save_stored_networks(int n)
{
    // Reallocate array to exact size
    free(scannedNetworks);
    scannedNetworks = (wifi_t *)malloc(n * sizeof(wifi_t));
    scannedCount = 0;                 
    if (!scannedNetworks) {
        return;
    }

    for (int i = 0; i < n; ++i) {
        String ssidStr = WiFi.SSID(i);     
        // Remove empty and dupes
        if (ssidStr.length() == 0 || ssid_exists(ssidStr.c_str()))
            continue;
        strncpy(scannedNetworks[scannedCount].ssid,
                ssidStr.c_str(),
                sizeof(scannedNetworks[scannedCount].ssid) - 1);
        scannedNetworks[scannedCount].ssid[
            sizeof(scannedNetworks[scannedCount].ssid) - 1] = '\0';
        scannedNetworks[i].connected = false;
        scannedNetworks[i].rssi = WiFi.RSSI(i);
        scannedNetworks[i].encryption = WiFi.encryptionType(i);
        scannedNetworks[i].channel = WiFi.channel(i);
        scannedCount++;
    }
    ui_print_wifi_scan();
}

const char *get_wifi_password_for_ssid(const char *ssid) 
{
    DynamicJsonDocument nets(WIFI_BYTES);
    read_JSON("/wifi.json", nets);
    Serial.println("Read json successfully");
    JsonArray networks = nets["networks"].as<JsonArray>();
    Serial.println("Network read success");
    serializeJson(nets, Serial);
    Serial.println();
    for (JsonObject net : networks) {
        if (ssid == net["ssid"])
            return net["password"];
    }
    return "";
}

static void connect_to_saved_wifi()
{
    DynamicJsonDocument nets(WIFI_BYTES);
    read_JSON("/wifi.json", nets);
    Serial.println("Read json successfully");
    JsonArray networks = nets["networks"].as<JsonArray>();
    Serial.println("Network read success");
    serializeJson(nets, Serial);
    Serial.println();
    if (scannedCount == 0 || !scannedNetworks) {
        Serial.printf("No networks_found: %d\n", scannedCount);
        return;
    }
    for (JsonObject net : networks) {
        for (size_t i = 0; i < scannedCount; i++) {
            Serial.println("Found network:");
            Serial.println(scannedNetworks[i].ssid);
            if (String(scannedNetworks[i].ssid) == net["ssid"]) {
                WiFi.begin(net["ssid"] | "", net["password"] | "");
                Serial.printf("Connecting to: %s %s\n", String(net["ssid"]).c_str(), String(net["password"]).c_str());
                WiFi.scanDelete();
                return;
            }
        }
    }
}

void start_wifi_scan()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanNetworks(true, false);
    Serial.println("Starting wifi scan"); 
    wifi_scanning = true;
    wifi_connection_attempts++;
}

static void check_wifi()
{
    Serial.println("Wifi check");
    if (!monitor.wifi_connected) {
        if (!saved_defined_network) {
            save_wifi_to_file(WIFI_SSID, WIFI_PASSWORD);
            saved_defined_network = true;
        }
        lv_style_set_text_color(&style_wifi, color_red);
        start_wifi_scan();
    } else {
        ui_print_wifi_scan();
        lv_style_set_text_color(&style_wifi, color_green);
        Serial.print("WiFi connected! IP: ");
        Serial.println(WiFi.localIP());
        // for (int i = 0; i < n; ++i) {
        if (!last_time_sync) {
            configTime(utc_offset_value*3600, 0, ntpServer);
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
    randomSeed(analogRead(0)); // Used for generating random strings
    pinMode(LEFT_BTN_PIN, INPUT_PULLUP);
    attachInterrupt(LEFT_BTN_PIN, handleLeftButtonPress, FALLING);
    instance.onEvent([](DeviceEvent_t event, void *params, void * user_data) {
        if (instance.getPMUEventType(params) == PMU_EVENT_KEY_CLICKED) {
            last_event = millis();
            if (monitor.sleeping)
                wakeup();
            else {
                switch_to_screen(CLOCK_SCREEN);
            }
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
    save_html_files_to_disk();
    hw_update_monitor();
    init_styles();
    init_screens();
    switch_to_screen(CLOCK_SCREEN);
    utc_offset_value = get_int_key_value("utc_offset", DEFAULT_UTC_OFFSET);
    utc2_offset_value = get_int_key_value("utc2_offset", DEFAULT_UTC2_OFFSET);
    longitude_value = get_float_key_value("longitude", DEFAULT_LONGITUDE_VALUE);
    latitude_value = get_float_key_value("latitude", DEFAULT_LATITUDE_VALUE);
    check_wifi();
    ui_alarm.hour = get_int_key_value("ui_alarm_hour", 0);
    ui_alarm.minute = get_int_key_value("ui_alarm_min", 0);
    ui_alarm.set = get_bool_key_value("ui_alarm_set", false);
}

void toggle_torch()
{
    if (!torch_active) {
        lv_scr_load(secondary_screens[TORCH_SCREEN]);
        torch_active = true;
    } else {
        switch_to_screen(CLOCK_SCREEN);
        torch_active = false;
    }
}

void loop()
{
    lv_timer_handler();
    instance.loop();

    if (monitor.wifi_ap_server)
        handle_clients();
    if (!monitor.sleeping) {
        int current_millis = millis();
        if (left_btn_pressed) {
            left_btn_pressed = false;
            if (current_screen == CLOCK_SCREEN) {
                toggle_torch();
            }
            Serial.println("Left btn pressed!");
        }

        // simple check for seconds (change to use lv_timer later)
        if (current_millis - last_millis >= ONE_SECOND) {
            last_millis = current_millis;
            update_time();
            if (current_screen == CLOCK_SCREEN) // Move to check if there's no valid date, or midnight
                update_date();
            if (wifi_scanning && monitor.wifi_enabled) {
                int result = WiFi.scanComplete();
                Serial.printf("Scan result %d\n", result);
                ui_update_wifi(result);
                if (result >= 0) {
                    save_stored_networks(result);
                    connect_to_saved_wifi();
                    wifi_scanning = false;
                }
            }
        }
        if (current_millis - last_status_check >= FIVE_SECONDS) {
            last_status_check = current_millis;
            hw_update_monitor();
            refresh_screen_headers();
            ui_refresh_sensor_labels();
        }
        if (current_millis - last_wifi_check >= ONE_MINUTE && monitor.wifi_enabled && !monitor.wifi_ap_server) {
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
    delay(5);
}