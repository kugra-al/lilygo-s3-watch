#include <Arduino.h>
#include <Preferences.h>

extern Preferences cache;

void putStringKV(const char *key, String value);
String getStringKV(const char *key, String default_value);