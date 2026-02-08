#include <WiFi.h>
#include <LilyGoLib.h>
#include "hw_monitor.h"

hw_monitor_t monitor;

void hw_update_monitor()
{
    monitor.wifi_connected = (WiFi.status() == WL_CONNECTED);
    monitor.battery_voltage = instance.pmu.getBattVoltage();
    monitor.battery_percent = instance.pmu.getBatteryPercent();
    monitor.usb_voltage = instance.pmu.getVbusVoltage();
    monitor.sys_voltage = instance.pmu.getSystemVoltage();
    monitor.charging = instance.pmu.isCharging();
    monitor.temperature = instance.pmu.getTemperature();
    monitor.ssid = WiFi.SSID().c_str();
    monitor.local_ip = WiFi.localIP().toString().c_str();
    monitor.gateway_ip = WiFi.gatewayIP().toString().c_str();
    monitor.orientation = instance.sensor.direction();
}


