#define SerialGPS  Serial1
#define GPS_DEFAULT_TEXT "-"
extern bool gps_manually_toggled;

void draw_gps_screen();
void update_gps_stats(unsigned long ms);
void toggle_gps();