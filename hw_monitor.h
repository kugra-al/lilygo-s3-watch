typedef struct {
    bool wifi_connected;
    int battery_percent;
    float battery_voltage;
    float usb_voltage;
    float sys_voltage;
    bool charging;
    float temperature;
    String ssid;
    String local_ip;
    String gateway_ip;
    int orientation;
    bool sleeping;
} hw_monitor_t;

extern hw_monitor_t monitor;
extern int last_event;
extern bool is_sleeping;

void hw_update_monitor();
void fake_sleep();
void wakeup();