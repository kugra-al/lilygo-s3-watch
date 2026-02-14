#include <Arduino.h>
#include <Preferences.h>

extern Preferences cache;

void putStringKV(const char *key, String value);
String getStringKV(const char *key, String default_value);
void put_bool_key_value(const char *key, bool value);
bool get_bool_key_value(const char *key, bool default_value);
void put_int_key_value(const char *key, int value);
int get_int_key_value(const char *key, int default_value);
void mount_file_system();