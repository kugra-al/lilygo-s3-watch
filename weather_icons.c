/*******************************************************************************
 * Size: 24 px
 * Bpp: 1
 * Opts: --bpp 1 --size 24 --no-compress --stride 1 --align 1 --font WeatherSymbols.ttf --symbols ABCDEFGHIJKLMNOP --format lvgl -o weather_icons.c
 ******************************************************************************/
// Font glyphs from https://www.dafont.com/weather-symbols.charmap
// Need a better one

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef WEATHER_ICONS
#define WEATHER_ICONS 1
#endif

#if WEATHER_ICONS

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0041 "A" */
    0x7, 0x80, 0x23, 0x1, 0x7, 0x4, 0x6, 0x70,
    0x5, 0x0, 0x18, 0x0, 0x70, 0x1, 0x40, 0x8,
    0xff, 0xc0, 0x30, 0x0, 0xc0, 0x3, 0x80, 0x4,
    0x0, 0x20, 0x0, 0x80,

    /* U+0042 "B" */
    0x0, 0x40, 0x18, 0x3, 0x0, 0xe0, 0x8, 0x4,
    0x1, 0x90, 0x76, 0xf, 0xe1, 0x98, 0x0, 0x11,
    0x26, 0x6c, 0xdd, 0xbb, 0xf3, 0x26,

    /* U+0043 "C" */
    0xa, 0x0, 0x1a, 0x0, 0x61, 0xe0, 0x1f, 0x30,
    0xde, 0x1c, 0x34, 0x6, 0x5c, 0x3, 0x10, 0x1,
    0x10, 0x1, 0x10, 0x1, 0x10, 0x2, 0xf, 0xfc,

    /* U+0044 "D" */
    0xa, 0x80, 0x39, 0x4, 0x90, 0xe4, 0xe3, 0xae,
    0x27, 0xc8, 0x1c, 0xf, 0xff, 0x32, 0x72, 0x92,
    0x89, 0xc8, 0x16, 0x0, 0x20, 0x0,

    /* U+0045 "E" */
    0x3c, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x34,
    0x76, 0x6e, 0xed, 0x6e, 0x7a, 0x3c,

    /* U+0046 "F" */
    0x1b, 0xfc, 0x0, 0xf, 0xc0, 0xdf, 0x8, 0x0,
    0x41, 0xe7, 0xf8, 0x6, 0x4, 0x0, 0x7f, 0x80,
    0xb, 0xe0, 0x1, 0x2, 0x0, 0x4b, 0xff, 0xf0,

    /* U+0047 "G" */
    0x0, 0x30, 0x1, 0x80, 0xc, 0x1, 0xf8, 0xc,
    0xe0, 0x41, 0xe1, 0x1, 0x9c, 0x3, 0x40, 0x6,
    0x0, 0x18, 0x0, 0x50, 0x2, 0x3f, 0xf0,

    /* U+0048 "H" */
    0x3c, 0x24, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34,
    0x76, 0x6e, 0xe5, 0x66, 0x7a, 0x3c,

    /* U+0049 "I" */
    0xf, 0xc3, 0xe0, 0x38, 0x7, 0x80, 0x70, 0xf,
    0x0, 0xf0, 0xf, 0x80, 0x78, 0x7, 0xc0, 0x3e,
    0x11, 0xfe, 0xf, 0x80,

    /* U+004A "J" */
    0x2, 0x0, 0xfc, 0xf, 0xf8, 0xff, 0xef, 0xff,
    0x81, 0x0, 0x8, 0x0, 0x40, 0x2, 0x0, 0x10,
    0x4, 0x80, 0x24, 0x0, 0xe0, 0x0,

    /* U+004B "K" */
    0x7, 0x80, 0x23, 0x1, 0x7, 0x4, 0x6, 0x70,
    0x5, 0x0, 0x18, 0x0, 0x70, 0x1, 0x40, 0x8,
    0xff, 0xc0, 0x31, 0x4, 0x8c, 0x37, 0xb0, 0x4,
    0x0, 0x20, 0x0, 0x80,

    /* U+004C "L" */
    0x3, 0x0, 0x36, 0x1, 0xc, 0x8, 0x1e, 0x60,
    0x1e, 0x0, 0x38, 0x0, 0xe0, 0x3, 0x80, 0x19,
    0xff, 0xc0, 0x0, 0x4, 0x80, 0x0, 0x0, 0x24,
    0x0, 0x0, 0x0, 0x80,

    /* U+004D "M" */
    0x3, 0x0, 0x33, 0x1, 0x84, 0x4, 0xe, 0x30,
    0xd, 0x0, 0x18, 0x0, 0x60, 0x1, 0x40, 0x8,
    0xff, 0xc0, 0x0, 0x2, 0x50, 0x1b, 0x60, 0x2d,
    0x0,

    /* U+004E "N" */
    0x1, 0x0, 0x24, 0xc0, 0xd2, 0x3, 0x19, 0x77,
    0x9c, 0x7f, 0x60, 0xfc, 0x3b, 0xf7, 0xf, 0xc0,
    0x5e, 0x67, 0x1, 0xc3, 0x59, 0xd, 0x20, 0x24,
    0xc0,

    /* U+004F "O" */
    0x0, 0x38, 0x0, 0x22, 0x3, 0xe0, 0xc1, 0x8,
    0x11, 0x3, 0x89, 0x80, 0x45, 0x0, 0x15, 0x0,
    0x8, 0x80, 0x4, 0x20, 0x4, 0xf, 0xfc, 0x0,

    /* U+0050 "P" */
    0x3, 0x0, 0x33, 0x1, 0x84, 0x4, 0xe, 0x30,
    0xd, 0x0, 0x1c, 0x0, 0x60, 0x1, 0x40, 0x8,
    0xff, 0xc0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 231, .box_w = 14, .box_h = 16, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 28, .adv_w = 172, .box_w = 11, .box_h = 16, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 269, .box_w = 16, .box_h = 12, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 74, .adv_w = 214, .box_w = 13, .box_h = 13, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 96, .adv_w = 115, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 110, .adv_w = 312, .box_w = 19, .box_h = 10, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 134, .adv_w = 236, .box_w = 14, .box_h = 13, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 157, .adv_w = 114, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 171, .adv_w = 206, .box_w = 12, .box_h = 13, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 191, .adv_w = 229, .box_w = 13, .box_h = 13, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 213, .adv_w = 233, .box_w = 14, .box_h = 16, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 241, .adv_w = 232, .box_w = 14, .box_h = 16, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 269, .adv_w = 231, .box_w = 14, .box_h = 14, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 294, .adv_w = 246, .box_w = 14, .box_h = 14, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 319, .adv_w = 279, .box_w = 17, .box_h = 11, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 343, .adv_w = 232, .box_w = 14, .box_h = 10, .ofs_x = 0, .ofs_y = 4}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 65, .range_length = 16, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};

extern const lv_font_t lv_font_montserrat_24;


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t weather_icons = {
#else
lv_font_t weather_icons = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .static_bitmap = 0,
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = &lv_font_montserrat_24,
#endif
    .user_data = NULL,
};



#endif /*#if WEATHER_ICONS*/
