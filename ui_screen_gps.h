#define SerialGPS  Serial1
#define GPS_DEFAULT_TEXT "-"

void draw_gps_screen();
void update_gps_stats(unsigned long ms);
void toggle_gps();