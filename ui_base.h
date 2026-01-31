typedef struct {
    int x, y;
    lv_align_t align;
    lv_text_align_t text_align;
} align_cfg_t;

typedef struct {
    int height, width;
} size_cfg_t;

#define LV_COLOR_TRANSP  lv_color_hex(0x00ff00) 

static lv_color_t color_green = lv_color_hex(0x39FF14);
static lv_color_t color_red = lv_color_hex(0xFF0000);
static lv_color_t color_grey = lv_color_hex(0x808080);
static lv_color_t color_yellow = lv_color_hex(0xFFFF00);
static lv_color_t color_default = color_green;

// New styles
extern lv_style_t style_default;
extern lv_style_t style_default_large;
extern lv_style_t style_default_medium;
extern lv_style_t style_default_small;
extern lv_style_t style_connected;
extern lv_style_t style_inactive;
extern lv_style_t style_disconnected;
extern lv_style_t style_roller;
extern lv_style_t style_roller_selected;
extern lv_style_t style_wifi;
extern lv_style_t style_charge;

void init_styles();
lv_obj_t *ui_add_aligned_label(char *cache_key, char *default_text, lv_obj_t *align_to_obj, lv_style_t *style, align_cfg_t *aligns, size_cfg_t *sizes, lv_obj_t *screen);
lv_obj_t *ui_add_button(char *cache_key, char *default_text, lv_obj_t *align_to_obj, lv_style_t *style, void (*callback)(lv_event_t *), 
  align_cfg_t *aligns, size_cfg_t *sizes, lv_obj_t *screen);
lv_obj_t *init_popup(char *label_text, char *btn_text, void (*callback)(lv_event_t *));