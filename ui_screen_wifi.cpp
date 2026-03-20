#include "ui_screens.h"

static lv_obj_t *wifi_password_text;
char wifi_ssid_selected[33];
char wifi_manager_ssid_selected[33];

lv_obj_t *wifi_scan_container;
lv_obj_t *wifi_input_box;
lv_obj_t *saved_networks_container;

void ui_update_wifi(int result)
{
    lv_obj_clean(wifi_scan_container);
    lv_obj_t *label = lv_label_create(wifi_scan_container);
    lv_label_set_text_fmt(label, "Result: %d\n", result);
}

static void wifi_item_event_cb(lv_event_t *e)
{
    lv_obj_t *clicked = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *list = lv_obj_get_parent(clicked);

    // Uncheck all siblings
    uint32_t child_cnt = lv_obj_get_child_cnt(list);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(list, i);
        if (child != clicked) {
            lv_obj_clear_state(child, LV_STATE_CHECKED);
        }
    }
    lv_obj_t *ssid_label = lv_obj_get_child(clicked, 0);
    if (ssid_label) {
        lv_obj_t * ssid_label = lv_obj_get_child(clicked, 0);
        const char *text = (const char *)lv_obj_get_user_data(clicked);
        strncpy(wifi_ssid_selected, text, 32);
        wifi_ssid_selected[32] = '\0';  // Force null termination

    }
    lv_obj_add_state(clicked, LV_STATE_CHECKED);
}

// Antenna on twatch is made of cheese, so boost these up a bit. -69 is right next to a router
int rssi_to_num(int rssi) {
    if (rssi >= -60) return 5;
    if (rssi >= -80) return 4;
    if (rssi >= -90) return 3;
    if (rssi >= -100) return 2;
    return 1;
}

// this crashes sometimes
void ui_print_wifi_scan() {
    lv_obj_clean(wifi_scan_container);
    
    lv_obj_t *list = lv_list_create(wifi_scan_container);
    lv_obj_align(list, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(list, &style_container, LV_PART_MAIN);
    lv_obj_set_width(list, INNER_CONTENT_WIDTH);
    for (size_t i = 0; i < scannedCount; i++) {
        char ssidBuf[64];
        snprintf(ssidBuf, sizeof(ssidBuf), "%s", scannedNetworks[i].ssid);
        if (strcmp(monitor.ssid.c_str(), scannedNetworks[i].ssid) == 0)
            snprintf(ssidBuf, sizeof(ssidBuf), "%s " LV_SYMBOL_WIFI, scannedNetworks[i].ssid);
        if (scannedNetworks[i].encryption != WIFI_AUTH_OPEN) {
            char temp[64];
            snprintf(temp, sizeof(temp), "%s %s", ssidBuf, LV_SYMBOL_WARNING);
            strncpy(ssidBuf, temp, sizeof(ssidBuf));
        }
        char temp[64];
        Serial.printf("Rssi for %s: %d\n", scannedNetworks[i].ssid, scannedNetworks[i].rssi);
        snprintf(temp, sizeof(temp), "%s %d*", ssidBuf, rssi_to_num(scannedNetworks[i].rssi));
        strncpy(ssidBuf, temp, sizeof(ssidBuf));
        ssidBuf[sizeof(ssidBuf)-1] = '\0';

        lv_obj_t *item = lv_list_add_btn(list, NULL, ssidBuf);
        lv_obj_add_flag(item, LV_OBJ_FLAG_CHECKABLE);
        lv_label_set_long_mode(lv_obj_get_child(item, 0), LV_LABEL_LONG_WRAP);
        lv_obj_add_style(item, &style_container, LV_PART_MAIN);
        lv_obj_add_style(item, &style_default_small, LV_PART_MAIN);
        lv_obj_set_style_bg_color(item, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(item, color_default, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(item, lv_color_black(), LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_add_event_cb(item, wifi_item_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_set_user_data(item, (void*)scannedNetworks[i].ssid);
    }
}

void kb_event_cb(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target_obj(e);

    if(code == LV_EVENT_CANCEL) {   
        lv_obj_delete(kb);                                    
        lv_obj_delete(wifi_input_box);
        return;
    }

    /* From msgbox buttons: VALUE_CHANGED when OK/Cancel clicked */
    if(code == LV_EVENT_READY) { 
        const char *wifi_password = lv_textarea_get_text(wifi_password_text);     
        Serial.printf("Pass: %s\n", wifi_password);  
        Serial.printf("SSID: %s\n", wifi_ssid_selected);
        WiFi.begin(wifi_ssid_selected, wifi_password);
        save_wifi_to_file(wifi_ssid_selected, wifi_password);            
        lv_obj_delete(kb);                                    
        lv_obj_delete(wifi_input_box);
        return;
    }
}

lv_obj_t *show_wifi_input_box()
{
    /* Modal message box */
    lv_obj_t *mbox = lv_msgbox_create(screens[current_screen]);                                            
    //lv_msgbox_add_text(mbox, "Password:");                     
    lv_obj_set_width(mbox, 220);
    /* Content area of msgbox */
    lv_msgbox_add_text(mbox, wifi_ssid_selected);
    lv_obj_t *content = lv_msgbox_get_content(mbox);                  
    lv_obj_add_style(mbox, &style_container, LV_PART_MAIN);
    lv_obj_add_style(mbox, &style_default_small, LV_PART_MAIN);
    /* Text area inside msgbox */
    wifi_password_text = lv_textarea_create(content);   
    lv_obj_add_style(wifi_password_text, &style_container, LV_PART_MAIN);                              
    lv_textarea_set_one_line(wifi_password_text, true);                               
    lv_obj_set_width(wifi_password_text, lv_pct(100));         

    lv_obj_align(mbox, LV_ALIGN_TOP_MID, 0, 40);
    return mbox;
}

void wifi_connect_button_cb(lv_event_t *e) 
{ 
    if (monitor.sleeping)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
        if (!strlen(wifi_ssid_selected))
            return;
        lv_obj_t *keyboard = lv_keyboard_create(screens[current_screen]);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_MAIN);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_PRESSED);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_FOCUSED);
        lv_obj_add_style(keyboard, &style_keyboard, LV_PART_ITEMS | LV_STATE_DISABLED);
        /* Send characters into our textarea */
        wifi_input_box = show_wifi_input_box();
        const char *saved_password = get_wifi_password_for_ssid(wifi_ssid_selected);
        if (saved_password)
            lv_textarea_set_text(wifi_password_text, saved_password);
        lv_keyboard_set_textarea(keyboard, wifi_password_text);
        lv_obj_add_event_cb(keyboard, kb_event_cb, LV_EVENT_ALL, NULL);
        Serial.println("Connect button clicked");
    }
}

void wifi_start_local_server_cb(lv_event_t *e)
{
    if (monitor.sleeping)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        wifi_setup_ap();
    }
}

static void wifi_manager_item_event_cb(lv_event_t *e)
{
    lv_obj_t *clicked = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *list = lv_obj_get_parent(clicked);
    Serial.println("Clicked");
    // Uncheck all siblings
    uint32_t child_cnt = lv_obj_get_child_cnt(list);
    for (uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(list, i);
        if (child != clicked) {
            lv_obj_clear_state(child, LV_STATE_CHECKED);
        }
    }
    Serial.println("After click");
    lv_obj_t *ssid_label = lv_obj_get_child(clicked, 0);
    if (ssid_label) {
        const char *text = lv_list_get_btn_text(saved_networks_container, clicked);
        Serial.printf("Selected: %s", text);
        strncpy(wifi_manager_ssid_selected, text, 32);
        wifi_manager_ssid_selected[32] = '\0';  // Force null termination

    }
    lv_obj_add_state(clicked, LV_STATE_CHECKED);
}

void draw_saved_networks()
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

    for (JsonObject net : networks) {
        Serial.printf("Drawing %s\n", net["ssid"]);
        if (!strlen(net["ssid"]))
            continue;
        lv_obj_t *item = lv_list_add_btn(saved_networks_container, NULL, net["ssid"]);
        lv_obj_add_flag(item, LV_OBJ_FLAG_CHECKABLE);
        lv_label_set_long_mode(lv_obj_get_child(item, 0), LV_LABEL_LONG_WRAP);
        lv_obj_add_style(item, &style_container, LV_PART_MAIN);
        lv_obj_add_style(item, &style_default_small, LV_PART_MAIN);
        lv_obj_set_style_bg_color(item, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(item, color_default, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(item, lv_color_black(), LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_add_event_cb(item, wifi_manager_item_event_cb, LV_EVENT_CLICKED, NULL);
        //lv_obj_set_user_data(item, net["password"]);
    }
}

void draw_wifi_manage_screen()
{
    lv_obj_t *screen = secondary_screens[WIFI_MANAGE_SCREEN];
    lv_obj_t *wifi_manage_title_label = ui_add_title_label("Saved Networks", screen);   
    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, wifi_manage_title_label, screen);
    saved_networks_container = lv_list_create(content);
    lv_obj_align(saved_networks_container, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(saved_networks_container, &style_container, LV_PART_MAIN);
    lv_obj_set_width(saved_networks_container, INNER_CONTENT_WIDTH);

    draw_saved_networks();
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *btn_container = ui_add_button_row(screen);
    lv_obj_t *settings_btn = ui_add_button(NULL, "Back", NULL, &style_default_small, back_button_cb, 
        &btn_align, &btn_size, btn_container);   
    btn_align.align = LV_ALIGN_TOP_MID;
    lv_obj_t *scan_btn = ui_add_button(NULL, "Edit", NULL, &style_default_small, back_button_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_RIGHT;
    lv_obj_t *connect_btn = ui_add_button(NULL, "Delete", NULL, &style_default_small, back_button_cb, 
        &btn_align, &btn_size, btn_container); 
}

void wifi_manage_button_cb(lv_event_t *e)
{
    if (monitor.sleeping)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {    
        lv_scr_load(secondary_screens[WIFI_MANAGE_SCREEN]);
    }
}

void draw_wifi_settings_screen()
{
    lv_obj_t *screen = secondary_screens[WIFI_SETTINGS_SCREEN];
    lv_obj_t *wifi_settings_title_label = ui_add_title_label("Wifi Settings", screen);   
    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, wifi_settings_title_label, screen);

    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 180};

    lv_obj_t *local_server_btn = ui_add_button(NULL, "Start Local Server", NULL, &style_default_small, wifi_start_local_server_cb, 
        &btn_align, &btn_size, content);  
    btn_align.y = 50;
    lv_obj_t *captive_portal_btn = ui_add_button(NULL, "Start Cap. Portal", NULL, &style_default_small, back_button_cb, 
        &btn_align, &btn_size, content);  

    btn_align.y = 0;
    btn_size.width = 80;
    lv_obj_t *btn_container = ui_add_button_row(screen);
    lv_obj_t *settings_btn = ui_add_button(NULL, "Back", NULL, &style_default_small, back_button_cb, 
        &btn_align, &btn_size, btn_container);    
    btn_align.align = LV_ALIGN_TOP_RIGHT;
    lv_obj_t *connect_btn = ui_add_button(NULL, "Manage", NULL, &style_default_small, wifi_manage_button_cb, 
        &btn_align, &btn_size, btn_container);
}

void wifi_scan_btn_cb(lv_event_t *e)
{
    if (monitor.sleeping)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED) {
       start_wifi_scan();
    }
}

void wifi_settings_btn_cb(lv_event_t *e)
{
    if (monitor.sleeping)
        return;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_scr_load(secondary_screens[WIFI_SETTINGS_SCREEN]);
    }
}

void draw_wifi_screen()
{
    lv_obj_t *screen = screens[WIFI_SCREEN];
    lv_obj_t *title = ui_add_title_label("Wifi", screen);
    wifi_scan_container = ui_add_content_container(CONTENT_HEIGHT_BUTTONS, title, screen);
    Serial.println("Drawing wifi screen");
    
    lv_obj_t *btn_container = ui_add_button_row(screen);
    align_cfg_t btn_align = {0, 0, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t btn_size = {40, 80};
    lv_obj_t *settings_btn = ui_add_button(NULL, LV_SYMBOL_SETTINGS, NULL, &style_default_small, wifi_settings_btn_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_MID;
    lv_obj_t *scan_btn = ui_add_button(NULL, LV_SYMBOL_REFRESH, NULL, &style_default_small, wifi_scan_btn_cb, 
        &btn_align, &btn_size, btn_container);
    btn_align.align = LV_ALIGN_TOP_RIGHT;
    lv_obj_t *connect_btn = ui_add_button(NULL, "Connect", NULL, &style_default_small, wifi_connect_button_cb, 
        &btn_align, &btn_size, btn_container);
}
