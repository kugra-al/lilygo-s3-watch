#define ONE_SECOND 1000
#define FIVE_SECONDS 5000
#define THIRTY_SECONDS 30000
#define ONE_MINUTE 60000
#define TWO_MINUTES ONE_MINUTE*2
#define THIRTY_MINUTES 1800000
#define TWELVE_HOURS 43200000

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
