// Handles cache and file sys functions

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <FFat.h>
#include "cache.h"

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
    DynamicJsonDocument doc(4096);

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

    // Check duplicate
    for (JsonObject net : networks) {
        if (strcmp(net["ssid"] | "", ssid) == 0) {
            Serial.println("Network already exists");
            return false;
        }
    }

    // Add new
    JsonObject net = networks.createNestedObject();
    net["ssid"] = ssid;
    net["password"] = password;

    Serial.println("Final JSON:");
    serializeJsonPretty(doc, Serial);
    Serial.println();

    return write_JSON(wifi_file, doc);
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
