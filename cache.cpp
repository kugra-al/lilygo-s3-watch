// Handles cache and file sys functions

#include <Arduino.h>
#include <FS.h>
#include <FFat.h>
#include "cache.h"

Preferences cache;

void mount_file_system()
{
    if (!FFat.begin(false)) {
        Serial.println("FFat mount failed");
    } else {
        Serial.println("FFat mount succeeded");
    }
}

String read_file(const char *path)
{
    String content = "";
    Serial.printf("Reading file: %s\n", path);
    File file = FFat.open(path, "r");
    if (!file) {
        Serial.printf("Failed to open file: %s\n", path);
        return content;
    }
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();
    return content;
}

bool write_file(const char *path, const char *content) 
{
    Serial.printf("Writing %s\n", path);
    File file = FFat.open(path, "w");
    if (!file) {
        Serial.printf("Failed to write to file: %s\n", path);
        return false;
    }
    file.print(content);
    file.close();
    return true;
}

bool file_exists(const char *path)
{
    return FFat.exists(path);
}

void put_string_key_value(const char* key, String value) {
    cache.begin(key, false);
    cache.putString(key, value);  
    cache.end();
}

String get_string_key_value(const char* key, String defaultVal = "") {
    cache.begin(key, true);
    String val = cache.getString(key, defaultVal);
    cache.end();
    return val;
}

void put_bool_key_value(const char *key, bool value)
{
    cache.begin(key, false);
    cache.putBool(key, value);
    cache.end();
}

bool get_bool_key_value(const char *key, bool defaultVal = false)
{
    cache.begin(key, true);
    bool val = cache.getBool(key, defaultVal);
    cache.end();
    return val;
}

void put_int_key_value(const char *key, int value)
{
    cache.begin(key, false);
    cache.putInt(key, value);
    cache.end();
}

int get_int_key_value(const char *key, int defaultVal = NULL)
{
    cache.begin(key, true);
    int val = cache.getInt(key, defaultVal);
    cache.end();
    return val;
}
