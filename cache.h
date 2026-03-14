#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include "nvs.h"
#include "nvs_flash.h"

extern Preferences cache;

void put_string_key_value(const char *key, String value);
String get_string_key_value(const char *key, String default_value);
void put_bool_key_value(const char *key, bool value);
bool get_bool_key_value(const char *key, bool default_value);
void put_int_key_value(const char *key, int value);
int get_int_key_value(const char *key, int default_value);
void put_float_key_value(const char *key, float value);
float get_float_key_value(const char *key, float defaultVal);
void mount_file_system();
String read_file(const char *path);
bool write_file(const char *path, const char *content);
bool file_exists(const char *path);
bool delete_file(const char *path);
bool write_JSON(const char *path, DynamicJsonDocument &doc);
bool read_JSON(const char *path, DynamicJsonDocument &doc);
bool save_wifi_to_file(const char *ssid, const char *password);
void nvs_full_reset();