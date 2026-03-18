extern char current_wifi_ssid[];
extern char current_wifi_password[];
extern bool saved_defined_network, wifi_scanning;

#define WIFI_BYTES 32768
typedef struct {
    char ssid[33];
    bool connected;
    int32_t  rssi;        // signal strength in dBm
    uint8_t  encryption;  // WiFi.encryptionType(...)
    uint8_t  channel;     // WiFi.channel(...)
} wifi_t;

extern wifi_t *scannedNetworks;
extern size_t  scannedCount;

void start_wifi_scan();
const char *get_wifi_password_for_ssid(const char *ssid);
void toggle_wifi();
void wifi_disable_ap();
void handle_clients();
void wifi_setup_ap();
void save_stored_networks(int n);
void connect_to_saved_wifi();