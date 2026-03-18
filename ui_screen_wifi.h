extern lv_obj_t *wifi_scan_container;
extern char current_wifi_ssid[];
extern char current_wifi_password[];

void draw_wifi_screen();
void draw_wifi_settings_screen();
void handle_clients();
void ui_update_wifi(int result);
void ui_print_wifi_scan();
void wifi_disable_ap();