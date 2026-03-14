#include <lvgl.h>
#include "ui_screen_cache.h"
#include "ui_screen_weather.h"
#include "HardwareSerial.h"
#include <LilyGoLib.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "cache.h"

extern lv_obj_t *time_label, *time_label_2;
extern lv_obj_t *date_label, *clock_temp_label, *clock_wind_label, *current_weather, *sun_status;
extern lv_obj_t *status_label;
extern lv_obj_t *wifi_label, *battery_label, *charge_label, *bluetooth_label, *gps_label, *alarm_symbol_label;
extern lv_obj_t *status_ssid_value_label, *status_local_ip_value_label, *status_gateway_ip_value_label, 
    *status_power_value_label, *status_temp_value_label, *status_vbus_value_label, *status_sysvolt_value_label, 
    *status_mem_value_label, *alarm_time_status_label, *status_ffat_value_label;
extern lv_obj_t *alarm_time_label, *alarm_hours_roller, *alarm_minutes_roller;
extern lv_obj_t *popup;
extern lv_obj_t *weather_screen_label, *weather_screen_status_label;
extern lv_obj_t *wifi_scan_container;
extern int current_screen;
extern int utc_offset_value, utc2_offset_value;
extern float longitude_value, latitude_value;

typedef struct {
    int hour;
    int minute;
    bool running = false;
    bool set = false;
    int end_time = 0;
} alarm_cfg_t;

extern alarm_cfg_t ui_alarm;
extern lv_obj_t *screens[];
#define NUM_SCREENS (sizeof(screens) / sizeof(screens[0]))
extern lv_obj_t *secondary_screens[];
#define NUM_SECONDARY_SCREENS (sizeof(secondary_screens) / sizeof(secondary_screens[0]))

#define CLOCK_SCREEN 0
#define STATUS_SCREEN 1
#define WEATHER_SCREEN 2
#define ALARM_SCREEN 3
#define WIFI_SCREEN 4

// Secondary screens
#define SETTINGS_SCREEN 0
#define CACHE_SCREEN 1

#define DEFAULT_UTC_OFFSET 0
#define DEFAULT_UTC2_OFFSET -1
#define DEFAULT_LONGITUDE_VALUE -0.1275
#define DEFAULT_LATITUDE_VALUE 51.5033

void draw_clock_screen();
void draw_status_screen();
void draw_alarm_screen();
void draw_wifi_screen();
void draw_settings_screen();
void ui_refresh_sensor_labels();
void ui_update_wifi(int result);
void ui_print_wifi_scan();
void init_screens();
void switch_to_screen(int screen);
void update_time();
void update_date();
void refresh_screen_headers();
void update_battery_percent();
void alarm_alert();
void alarm_start();
void alarm_stop();
void back_button_cb(lv_event_t *e);