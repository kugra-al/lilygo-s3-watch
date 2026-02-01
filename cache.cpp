#include <Arduino.h>
#include "cache.h"

Preferences cache;

void putStringKV(const char* key, String value) {
    cache.begin(key, false);
    cache.putString(key, value);  // Store "public_ip" -> "8.8.8.8"
    cache.end();
}

String getStringKV(const char* key, String defaultVal = "") {
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