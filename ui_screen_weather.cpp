#include "ui_screens.h"

lv_obj_t *weather_screen_label, *weather_screen_status_label;

String wind_dir_to_text(float deg) {
  // normalize
  if (deg < 0) deg += 360;
  if (deg >= 360) deg -= 360;

  static const char* dirs[] = {"N","NE","E","SE","S","SW","W","NW","N"};
  int index = (int)round(deg / 45.0);   // 360 / 8 = 45°
  return String(dirs[index]);
}

char *get_weather_icon(int code) 
{
    if (code >= 0 && code <= 1)
        return "N"; // Sunny
    if (code >= 2 && code <= 3)
        return "C"; // Partly cloudy
    if (code >= 4 && code <= 48)
        return "P"; // Cloudy
    if (code >= 51 && code <= 55 || code == 61 || code >= 80 && code <= 81)
        return "M"; // Drizzle
    if (code >= 56 && code <= 57 || code >= 66 && code <= 71 || code >= 85 && code <= 86)
        return "L"; // Freezing drizzle/light snow
    if (code >= 63 && code <= 65 || code == 82)
        return "B"; // Rain
    if (code >= 73 && code <= 77)
        return "D"; // Snow
    if (code == 95)
        return "A"; // Thunderstorm
    if (code >= 96 && code <= 99)
        return "K"; // Thunderstorm with rain
    return "J";
}

void update_weather()
{
    Serial.println("Attempting to fetch weather");
    char longitude_str[16];
    sprintf(longitude_str, "%.4f", longitude_value);
    char latitude_str[16];
    sprintf(latitude_str, "%.4f", latitude_value);
    String url = String("https://api.open-meteo.com/v1/forecast?latitude=")+latitude_str+
        "&longitude="+longitude_str+"&current_weather=true&daily=sunrise,sunset,weather_code,temperature_2m_max,"+
        "temperature_2m_min&timezone=Europe/Vilnius&forecast_days=14";  
    Serial.println("Fetching url:");
    Serial.println(url);
    HTTPClient http;
    http.begin(url);
    int code = http.GET();
    if (code == 200) {
        Serial.println("Fetched weather");
        String payload = http.getString();
        StaticJsonDocument<2048> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
            float temp = doc["current_weather"]["temperature"];
            float wind = doc["current_weather"]["windspeed"];
            String windDir = wind_dir_to_text(doc["current_weather"]["winddirection"]); 
            String sunrise = doc["daily"]["sunrise"][0];
            String sunset = doc["daily"]["sunset"][0];
            int weatherCode = doc["current_weather"]["weathercode"];
           // String weatherIcon = get_weather_icon(weatherCode);
            int tPos = sunrise.indexOf('T');
            sunrise = sunrise.substring(tPos+1, tPos+6);
            sunset = sunset.substring(tPos+1, tPos+6);
            Serial.println("Temp: " + String(temp) + "°C");
            
            lv_label_set_text_fmt(clock_temp_label, "%.1f°C", temp);
            lv_label_set_text_fmt(clock_wind_label, "%.1fkm/h %s", wind, windDir.c_str());
            lv_label_set_text_fmt(current_weather, "%s", String(get_weather_icon(weatherCode)).c_str());   

            char cacheBuf[24];
            snprintf(cacheBuf, sizeof(cacheBuf),
                "Rise: %s Set: %s", sunrise, sunset);
            put_string_key_value("suntimes", cacheBuf);
            lv_label_set_text(sun_status, cacheBuf);
            JsonArray timeArr = doc["daily"]["time"];
            JsonArray tempMinArr = doc["daily"]["temperature_2m_min"];
            JsonArray tempMaxArr = doc["daily"]["temperature_2m_max"];
            JsonArray codeArr = doc["daily"]["weather_code"];
           // lv_obj_add_flag(weather_screen_label, LV_OBJ_FLAG_HIDDEN);
            align_cfg_t weather_align = {0, 25, LV_ALIGN_TOP_LEFT, LV_TEXT_ALIGN_AUTO};
            align_cfg_t weather_icon_align = {25, 0, LV_ALIGN_RIGHT_MID, LV_TEXT_ALIGN_AUTO};
            char weatherBuf[512];
            char weatherStatusBuf[64];
            for (int i = 0; i < 7; i++) {
                String timeStr = String(timeArr[i]);
                float tempMin = tempMinArr[i].as<float>();
                float tempMax = tempMaxArr[i].as<float>();
                char *code = get_weather_icon(codeArr[i].as<int>());
                weather_align.y += 20;
                char textBuf[64];
                char statusBuf[4];
                snprintf(textBuf, sizeof(textBuf), "%s %.0fc/%.0fc\n", timeStr.c_str(), tempMax, tempMin);
                strcat(weatherBuf, textBuf);
                snprintf(statusBuf, sizeof(statusBuf), "%s\n", String(code).c_str());
                strcat(weatherStatusBuf, statusBuf);
            }
            lv_label_set_text(weather_screen_label, String(weatherBuf).c_str());
            lv_label_set_text(weather_screen_status_label, String(weatherStatusBuf).c_str());
        }
    }
    http.end();
}

void draw_weather_screen()
{
    Serial.println("Drawing weather screen");
    lv_obj_t *screen = screens[WEATHER_SCREEN];
    lv_obj_t *weather_title_label = ui_add_title_label("Weather", screen);
    lv_obj_t *content = ui_add_content_container(CONTENT_HEIGHT, weather_title_label, screen);
    align_cfg_t weather_screen_align = {0, 0, LV_ALIGN_OUT_BOTTOM_LEFT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t weather_screen_size = {140, 180};
    weather_screen_label = ui_add_aligned_label(NULL, "No weather data", NULL, &style_default_small, &weather_screen_align, 
        &weather_screen_size, content);
    align_cfg_t weather_screen_status_align = {20, 0, LV_ALIGN_TOP_RIGHT, LV_TEXT_ALIGN_AUTO};
    size_cfg_t weather_screen_status_size = {160, 40};
    weather_screen_status_label = ui_add_aligned_label(NULL, "", weather_screen_label, &style_weather, &weather_screen_status_align, 
        &weather_screen_status_size, content);
}