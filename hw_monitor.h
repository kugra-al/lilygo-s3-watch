typedef struct {
    bool wifi_connected;
    float battery_percent;
    float battery_voltage;
} hw_monitor_t;

extern hw_monitor_t monitor;

void hw_update_monitor();