/*******************************************************************************
 * Size: 20 px
 * Bpp: 1
 * Opts: --bpp 1 --size 20 --no-compress --stride 1 --align 1 --font WeatherSymbols.ttf --symbols ABCDEFGHIJKLMNOP --format lvgl -o weather_icons.c
 ******************************************************************************/

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
    0x6, 0x1, 0x90, 0x10, 0xe3, 0x3, 0x40, 0x18,
    0x1, 0x40, 0x37, 0xfe, 0x0, 0x0, 0x60, 0x7,
    0x0, 0x20, 0x4, 0x0, 0x0,

    /* U+0042 "B" */
    0x1, 0x1, 0x3, 0x3, 0x0, 0x9, 0x1b, 0x1b,
    0x0, 0x49, 0xdb, 0xdb, 0xdb,

    /* U+0043 "C" */
    0x34, 0x1, 0xfc, 0x3e, 0x18, 0x50, 0x65, 0x0,
    0x90, 0x4, 0x80, 0x24, 0x2, 0x1f, 0xf0,

    /* U+0044 "D" */
    0x4, 0x5, 0xd1, 0x93, 0x1a, 0xc4, 0xe4, 0x1c,
    0x1d, 0x73, 0x26, 0x2e, 0x80, 0x80,

    /* U+0045 "E" */
    0x72, 0x94, 0xa5, 0x29, 0xdb, 0xde, 0xfa, 0xe0,

    /* U+0046 "F" */
    0x17, 0xf0, 0x2f, 0x98, 0x1, 0xe, 0x5f, 0x3,
    0xfc, 0x1, 0x8, 0x1, 0x7c, 0x2, 0x7f, 0xfe,

    /* U+0047 "G" */
    0x0, 0xc0, 0x18, 0x1, 0x80, 0xf4, 0x10, 0xe3,
    0x2, 0x40, 0x18, 0x1, 0x40, 0x37, 0xfe,

    /* U+0048 "H" */
    0x72, 0x9c, 0xe7, 0x39, 0xdb, 0xde, 0xfe, 0xe0,

    /* U+0049 "I" */
    0x1f, 0xe, 0x7, 0x1, 0xc0, 0xf0, 0x3c, 0xf,
    0x1, 0xe0, 0x3c, 0x47, 0xe0, 0x0,

    /* U+004A "J" */
    0xc, 0x7, 0xf1, 0xff, 0x7f, 0xf2, 0x48, 0x8,
    0x1, 0x0, 0x20, 0x4, 0x2, 0x80, 0x20, 0x0,

    /* U+004B "K" */
    0x6, 0x1, 0x90, 0x10, 0xe3, 0x3, 0x40, 0x18,
    0x1, 0x40, 0x37, 0xfe, 0x0, 0x1, 0x64, 0x37,
    0x40, 0x24, 0x4, 0x0, 0x0,

    /* U+004C "L" */
    0x1e, 0x4, 0x20, 0x83, 0x60, 0x18, 0x3, 0x0,
    0xdf, 0xf0, 0x0, 0x15, 0x1, 0x40, 0x0, 0x2,
    0x0,

    /* U+004D "M" */
    0x6, 0x1, 0x90, 0x10, 0xe3, 0x3, 0x40, 0x14,
    0x1, 0x40, 0x33, 0xfe, 0x8, 0x1, 0xa8, 0x1e,
    0x80,

    /* U+004E "N" */
    0x4, 0x1, 0x4c, 0x18, 0x86, 0xfb, 0x3f, 0xee,
    0xff, 0xf, 0x80, 0x70, 0x70, 0x74, 0xc8, 0x1c,
    0xc0, 0x0,

    /* U+004F "O" */
    0x0, 0xe0, 0xc, 0x30, 0x64, 0x18, 0x87, 0x13,
    0x3, 0x28, 0x3, 0xa0, 0x4, 0x20, 0x18, 0x7f,
    0xe0,

    /* U+0050 "P" */
    0x6, 0x1, 0x90, 0x10, 0xe3, 0x3, 0x40, 0x18,
    0x1, 0x40, 0x37, 0xfe
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 192, .box_w = 12, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 21, .adv_w = 143, .box_w = 8, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 224, .box_w = 13, .box_h = 9, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 49, .adv_w = 179, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 63, .adv_w = 96, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 71, .adv_w = 260, .box_w = 16, .box_h = 8, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 87, .adv_w = 197, .box_w = 12, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 102, .adv_w = 95, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 110, .adv_w = 172, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 124, .adv_w = 191, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 140, .adv_w = 194, .box_w = 12, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 193, .box_w = 11, .box_h = 12, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 178, .adv_w = 193, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 195, .adv_w = 205, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 213, .adv_w = 233, .box_w = 15, .box_h = 9, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 230, .adv_w = 193, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = 3}
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

extern const lv_font_t lv_font_montserrat_20;


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
    .line_height = 14,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .static_bitmap = 0,
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = &lv_font_montserrat_20,
#endif
    .user_data = NULL,
};



#endif /*#if WEATHER_ICONS*/
