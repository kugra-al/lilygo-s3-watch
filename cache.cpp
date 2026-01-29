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