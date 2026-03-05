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
    float freemem;
    size_t disk_bytes;
    size_t disk_used_bytes;
    bool wifi_enabled;
    bool bluetooth_enabled;
    bool gps_enabled;
} hw_monitor_t;

extern hw_monitor_t monitor;
extern int last_event;
extern bool is_sleeping;
extern bool wifi_enabled, bluetooth_enabled, gps_enabled;

void hw_update_monitor();
void fake_sleep();
void wakeup();