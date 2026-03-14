// Handles cache and file sys functions

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <FFat.h>
#include "cache.h"
#include "watch.h"
#include "ui_screens.h"

Preferences cache;

void mount_file_system()
{
    Serial.printf("Total bytes: %d\n", FFat.totalBytes());
//    if (!FFat.begin(true, "/ffat")) { // Uncomment this line and comment out next line on first run
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

bool delete_file(const char *path) {
    Serial.printf("Deleting file: %s\n", path);
    if (FFat.remove(path)) {           
        Serial.println("File deleted");
        return true;
    } else {
        Serial.println("Delete failed");
        return false;
    }
}

bool write_JSON(const char *path, DynamicJsonDocument &doc) {
  String jsonText;
  serializeJson(doc, jsonText);
  
  return write_file(path, jsonText.c_str());
}

bool read_JSON(const char *path, DynamicJsonDocument &doc) 
{
    String jsonText = read_file(path);

    if (jsonText.length() == 0) {
        doc.clear();
        doc.to<JsonObject>();
        return false;
    }

    DeserializationError err = deserializeJson(doc, jsonText);
    if (err) {
        Serial.println(err.c_str());
        doc.clear();
        doc.to<JsonObject>();
        return false;
    }

    return true;
}

bool save_wifi_to_file(const char *ssid, const char *password) 
{
    const char *wifi_file = "/wifi.json";
    DynamicJsonDocument doc(WIFI_BYTES);

    // Read existing
    if (file_exists(wifi_file)) {
        read_JSON(wifi_file, doc);
    }

    // Guarantee root object
    if (!doc.is<JsonObject>()) {
        doc.clear();
        doc.to<JsonObject>();
    }

    // Get or create array
    JsonArray networks;
    if (doc["networks"].is<JsonArray>()) {
        networks = doc["networks"].as<JsonArray>();
    } else {
        networks = doc.createNestedArray("networks");
    }

    // Try update existing
    for (JsonObject net : networks) {
        if (strcmp(net["ssid"] | "", ssid) == 0) {
            net["password"] = password;              // update here
            Serial.println("Network updated");
            Serial.println("Final JSON:");
            serializeJsonPretty(doc, Serial);
            Serial.println();
            return write_JSON(wifi_file, doc);       // overwrite file with updated doc
        }
    }

    // Add new (not found)
    JsonObject net = networks.createNestedObject();
    net["ssid"] = ssid;
    net["password"] = password;

    Serial.println("Network added");
    Serial.println("Final JSON:");
    serializeJsonPretty(doc, Serial);
    Serial.println();

    return write_JSON(wifi_file, doc);
}

void put_string_key_value(const char* key, String value) {
    cache.begin("settings", false);
    cache.putString(key, value);  
    cache.end();
    update_cache_stats();
}

String get_string_key_value(const char* key, String defaultVal = "") {
    cache.begin("settings", true);
    String val = cache.getString(key, defaultVal);
    cache.end();
    return val;
}

void put_bool_key_value(const char *key, bool value)
{
    cache.begin("settings", false);
    cache.putBool(key, value);
    cache.end();
    update_cache_stats();
}

bool get_bool_key_value(const char *key, bool defaultVal = false)
{
    cache.begin("settings", true);
    bool val = cache.getBool(key, defaultVal);
    cache.end();
    return val;
}

void put_int_key_value(const char *key, int value)
{
    cache.begin("settings", false);
    cache.putInt(key, value);
    cache.end();
    update_cache_stats();
}

int get_int_key_value(const char *key, int defaultVal = 0)
{
    cache.begin("settings", true);
    int val = cache.getInt(key, defaultVal);
    cache.end();
    return val;
}

void put_float_key_value(const char *key, float value)
{
    cache.begin("settings", false);
    cache.putFloat(key, value);
    cache.end();
    update_cache_stats();
}

float get_float_key_value(const char *key, float defaultVal = 0)
{
    cache.begin("settings", true);
    float val = cache.getFloat(key, defaultVal);
    cache.end();
    return val;
}