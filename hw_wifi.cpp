#include "ui_screens.h"

bool saved_defined_network = false, wifi_scanning = false;
wifi_t *scannedNetworks = nullptr;
size_t  scannedCount    = 0;
char current_wifi_ssid[33];
char current_wifi_password[65];
WiFiServer server(80);

void wifi_disable_ap()
{
    WiFi.softAPdisconnect(true);
    wifi_ap_server = false;
    Serial.println("Wifi AP disabled");
}

void generate_random_alphanum(char *str, size_t len) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t charset_len = strlen(charset);

    for (size_t i = 0; i < len; ++i) {
        str[i] = charset[random(charset_len)];
    }
    str[len] = '\0';
}

bool get_current_wifi(char* ssid, char* pass, size_t max_len) {
    if (!monitor.wifi_connected) {
        strcpy(ssid, "");
        strcpy(pass, "");
        return false;
    }
    
    wifi_config_t conf;
    if (esp_wifi_get_config(WIFI_IF_STA, &conf) == ESP_OK) {
        strncpy((char*)ssid, (char*)conf.sta.ssid, max_len - 1);
        ssid[max_len - 1] = '\0';
        strncpy((char*)pass, (char*)conf.sta.password, max_len - 1);
        pass[max_len - 1] = '\0';
        return true;
    }
    return false;
}

void wifi_setup_ap()
{
    Serial.println("Starting wifi ap..");

    // If there's no cached wifi_ap details, create new
    char wifi_ap_ssid[9]; 
    char wifi_ap_password[9]; 
    String ssid_str = get_string_key_value("wifi_ap_ssid", "");
    String pass_str = get_string_key_value("wifi_ap_pass", "");
    ssid_str.toCharArray(wifi_ap_ssid, sizeof(wifi_ap_ssid));
    pass_str.toCharArray(wifi_ap_password, sizeof(wifi_ap_password));

    if (!strlen(wifi_ap_ssid) || !strlen(wifi_ap_password)) {
        generate_random_alphanum(wifi_ap_ssid, 8);
        generate_random_alphanum(wifi_ap_password, 8);
        put_string_key_value("wifi_ap_ssid", wifi_ap_ssid);
        put_string_key_value("wifi_ap_pass", wifi_ap_password);
    }
    // If already connected, change mode and reconnect
    if (monitor.wifi_connected) {
        if (get_current_wifi(current_wifi_ssid, current_wifi_password, sizeof(current_wifi_ssid))) {
            // Have to disconnect first because we're likely already in WIFI_STA mode
            WiFi.disconnect(true);
            delay(1000);
            WiFi.mode(WIFI_MODE_APSTA);
            WiFi.begin(current_wifi_ssid, current_wifi_password);
            Serial.printf("Connecting to: %s %s\n", current_wifi_ssid, current_wifi_password);
        }
    }
    WiFi.softAP(wifi_ap_ssid, wifi_ap_password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);  // Usually 192.168.4.1
    char popup_text[256];
    snprintf(popup_text, sizeof(popup_text),
        "WiFi AP Started.\n"
        "SSID: %s\n"
        "Password: %s\n"
        "IP: %s",
        wifi_ap_ssid, wifi_ap_password, IP.toString().c_str());

    lv_obj_t *mbox = ui_show_popup(popup_text, lv_scr_act());

    server.begin();
    wifi_ap_server = true;
    Serial.println("Server started - connect to Watch-AP!");
}

void handle_clients() {
    WiFiClient client = server.available();  // Check for new clients
    
    if (client) {
        Serial.println("New client connected!");
        
        String request = "";
        while (client.connected()) {
            if (client.available()) {
                String line = client.readStringUntil('\n');
                request += line;
                if (line == "\r") break;  // End of headers
            }
        }
        
        if (file_exists(SETTINGS_HTML)) {
            // Send HTML response
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            String settings_html = read_file(SETTINGS_HTML);
            client.println(settings_html);
        } else {
            client.println("HTTP/1.1 404 Not Found");
            client.println("Content-type:text/plain");
            client.println();
            client.println("404: File Not Found");
        }
        
        client.stop();
        Serial.println("Client disconnected");
    }
}

void toggle_wifi()
{
    if (monitor.wifi_enabled) {
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        wifi_enabled = false;
        wifi_disable_ap();
    } else {
        start_wifi_scan();
        wifi_enabled = true;
    }
}

bool ssid_exists(const char *ssid) {
    for (size_t i = 0; i < scannedCount; ++i) {
        if (strcmp(scannedNetworks[i].ssid, ssid) == 0) {
            return true;
        }
    }
    return false;
}

void save_stored_networks(int n)
{
    // Reallocate array to exact size
    free(scannedNetworks);
    scannedNetworks = (wifi_t *)malloc(n * sizeof(wifi_t));
    scannedCount = 0;                 
    if (!scannedNetworks) {
        return;
    }

    for (int i = 0; i < n; ++i) {
        String ssidStr = WiFi.SSID(i);     
        // Remove empty and dupes
        if (ssidStr.length() == 0 || ssid_exists(ssidStr.c_str()))
            continue;
        strncpy(scannedNetworks[scannedCount].ssid,
                ssidStr.c_str(),
                sizeof(scannedNetworks[scannedCount].ssid) - 1);
        scannedNetworks[scannedCount].ssid[
            sizeof(scannedNetworks[scannedCount].ssid) - 1] = '\0';
        scannedNetworks[i].connected = false;
        scannedNetworks[i].rssi = WiFi.RSSI(i);
        scannedNetworks[i].encryption = WiFi.encryptionType(i);
        scannedNetworks[i].channel = WiFi.channel(i);
        scannedCount++;
    }
    ui_print_wifi_scan();
}

const char *get_wifi_password_for_ssid(const char *ssid) 
{
    DynamicJsonDocument nets(WIFI_BYTES);
    read_JSON("/wifi.json", nets);
    Serial.println("Read json successfully");
    JsonArray networks = nets["networks"].as<JsonArray>();
    Serial.println("Network read success");
    serializeJson(nets, Serial);
    Serial.println();
    for (JsonObject net : networks) {
        if (ssid == net["ssid"])
            return net["password"];
    }
    return "";
}

void connect_to_saved_wifi()
{
    DynamicJsonDocument nets(WIFI_BYTES);
    read_JSON("/wifi.json", nets);
    Serial.println("Read json successfully");
    JsonArray networks = nets["networks"].as<JsonArray>();
    Serial.println("Network read success");
    serializeJson(nets, Serial);
    Serial.println();
    if (scannedCount == 0 || !scannedNetworks) {
        Serial.printf("No networks_found: %d\n", scannedCount);
        return;
    }
    for (JsonObject net : networks) {
        for (size_t i = 0; i < scannedCount; i++) {
            Serial.println("Found network:");
            Serial.println(scannedNetworks[i].ssid);
            if (String(scannedNetworks[i].ssid) == net["ssid"]) {
                WiFi.begin(net["ssid"] | "", net["password"] | "");
                Serial.printf("Connecting to: %s %s\n", String(net["ssid"]).c_str(), String(net["password"]).c_str());
                WiFi.scanDelete();
                return;
            }
        }
    }
}

void start_wifi_scan()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanNetworks(true, false);
    Serial.println("Starting wifi scan"); 
    wifi_scanning = true;
}
