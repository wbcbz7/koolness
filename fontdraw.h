#pragma once
#include <stdint.h>
#include "bmpload.h"

struct font_descriptor_t {
    uint16_t offset;        // in atlas
    int8_t   xofs, yofs;    // in pixels
    uint8_t  width, height;
    uint8_t  advance;
    uint8_t  reserved;
};

struct font_info_t {
    int atlas_width, atlas_height;
    int start_char;
    font_descriptor_t *desc;
    uint32_t *atlasData;        // interleaved dwords pixels/mask
    BITMAPV5HEADER bmphead;
};

// init font info
int font_load(font_info_t *info, font_descriptor_t *desc, const char *bitmapFile, int startChar);

// unload font
int font_free(font_info_t *info);

// get character descriptor
font_descriptor_t* font_get_char_info(font_info_t *info, int ch);

// get length of string
int font_get_length(font_info_t *font, const char *str);

// draw one char
int font_draw_char(uint8_t *dst, struct font_info_t *font, int ch, int x, int y, int dst_pitch, uint32_t colormask = 0xFFFFFFFF);

// draw string
int font_draw_string(uint8_t *dst, struct font_info_t *font, const char *str, int x, int y, int dst_pitch, uint32_t colormask = 0xFFFFFFFF);

// draw one char
int font_draw_char_add(uint8_t *dst, struct font_info_t *font, int ch, int x, int y, int dst_pitch, uint32_t colormask = 0xFFFFFFFF);

// draw string
int font_draw_string_add(uint8_t *dst, struct font_info_t *font, const char *str, int x, int y, int dst_pitch, uint32_t colormask = 0xFFFFFFFF);


void font_generate_mask(uint32_t* dst, uint8_t* src, int l);

// ------------------
// scroller!
struct scroller_t {
    font_info_t *font;
    const char *text;

    // internals
    const char *curtext;
    int length;
    int textofs;        // in characters
    int pixelpos;       // in pixels
    int glyphpos;       // in pixels
};

// init scroller
void scroller_init(scroller_t *scroll);

// advance scroller
void scroller_advance(scroller_t *scroll, int dx);

// draw scroller from current position
void scroller_draw(scroller_t *scroll, uint8_t *dst, int x, int y, int width, int dst_pitch, uint32_t color = 0xFFFFFFFF);
