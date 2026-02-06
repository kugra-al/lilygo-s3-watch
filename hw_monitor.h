typedef struct {
    bool wifi_connected;
    int battery_percent;
    float battery_voltage;
    float usb_voltage;
    float sys_voltage;
    bool charging;
    float temperature;
} hw_monitor_t;

extern hw_monitor_t monitor;

void hw_update_monitor();