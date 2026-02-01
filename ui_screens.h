extern lv_obj_t *time_label, *time_label_2;
extern lv_obj_t *date_label, *outside_weather, *sun_status;
extern lv_obj_t *status_label;
extern lv_obj_t *wifi_label, *battery_label, *charge_label, *bluetooth_label, *gps_label, *alarm_symbol_label;
extern lv_obj_t *wifi_status_label, *power_status_label;
extern lv_obj_t *alarm_time_label, *alarm_hours_roller, *alarm_minutes_roller;
extern lv_obj_t *popup;

extern int current_screen;

typedef struct {
    int hour;
    int minute;
    bool running = false;
    bool set = false;
    int end_time = 0;
} alarm_cfg_t;

extern alarm_cfg_t ui_alarm;
static lv_obj_t *screens[3];
#define NUM_SCREENS (sizeof(screens) / sizeof(screens[0]))

#define CLOCK_SCREEN 0
#define STATUS_SCREEN 1
#define ALARM_SCREEN 2

void draw_clock_screen();
void draw_status_screen();
void draw_alarm_screen();
void draw_numpad();
int get_battery_percent_remaining();
void init_screens();
void switch_to_screen(int screen);
void update_time();
void update_date();
void refresh_screen_headers();
void update_weather();
void update_battery_percent();
void alarm_alert();
void alarm_start();
void alarm_stop();