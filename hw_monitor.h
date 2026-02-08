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
} hw_monitor_t;

extern hw_monitor_t monitor;

void hw_update_monitor();