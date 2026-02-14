// Handles hardware monitor and sleep/wake functions

#include "LilyGoWatchS3.h"
#include <WiFi.h>
#include <LilyGoLib.h>
#include "hw_monitor.h"

hw_monitor_t monitor;
int last_event = 0;
bool is_sleeping = false;

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
    monitor.sleeping = is_sleeping;
}

// Here we just dim the screen and block some loops. lightSleep and sleep both have some problems with what 
// they block and waking up correctly. At some point, do some battery readings so we can see if this 
// is good enough or not
void fake_sleep()
{
    instance.setBrightness(DEVICE_MIN_BRIGHTNESS_LEVEL);
    is_sleeping = true;
    Serial.println("Faking sleep");
    hw_update_monitor();
}

void wakeup()
{
    instance.setBrightness(DEVICE_MAX_BRIGHTNESS_LEVEL);
    is_sleeping = false;
    Serial.println("Waking up");
    hw_update_monitor();
}
