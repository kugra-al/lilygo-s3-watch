#include <WiFi.h>
#include <LilyGoLib.h>
#include "hw_monitor.h"

hw_monitor_t monitor = {false, 0, 0};

void hw_update_monitor()
{
    monitor.wifi_connected = (WiFi.status() == WL_CONNECTED);
    monitor.battery_voltage = instance.pmu.getBattVoltage();
    monitor.battery_percent = instance.pmu.getBatteryPercent();
}


