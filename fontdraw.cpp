#include <stdlib.h>
#include <string.h>
#include "fontdraw.h"
#include "bmpload.h"

struct drawglyph_t {
    uint32_t width, height;
    uint32_t dst_pitch, glyph_pitch;
    uint32_t dst_fixup, glyph_fixup;
    uint32_t colormask;
};

int font_get_length(font_info_t *font, const char *str) {
    if (str == NULL) return 0;
    int length = 0;
    while (*str != 0) {
        char ch = *str - font->start_char;
        struct font_descriptor_t *desc = font->desc + ch;
        length += desc->advance;
        str++;
    }
    return length;
}

font_descriptor_t* font_get_char_info(font_info_t *font, int ch) {
    return (ch < font->start_char) ? NULL : font->desc + (ch - font->start_char);
}

// FLAT image only (pitch == width)
void font_generate_mask(uint32_t* dst, uint8_t* src, int l) {
    for (int i = 0; i < l; i++) {
        *(dst+0) = *(uint32_t*)(src+0);
        *(dst+1) = ~(
            (*(src+0) ? 0x000000FF : 0) |
            (*(src+1) ? 0x0000FF00 : 0) |
            (*(src+2) ? 0x00FF0000 : 0) |
            (*(src+3) ? 0xFF000000 : 0)
        );
        src += 4; dst += 2;
    }
}

// init font info
int font_load(font_info_t *info, font_descriptor_t *desc, const char *bitmapFile, int startChar) {
    if ((info == NULL) || (desc == NULL) || (bitmapFile == NULL)) return 1;      // null pointers

    // load atlas
    // NB: palette is NOT loaded - make sure all fonts share the same palette!
    BITMAPV5HEADER bmphead;
    if (bmp_load8_header(bitmapFile, &bmphead) != 0) {
        return 1;
    }
    uint8_t* tempgfx = new uint8_t[bmphead.bV5Width * bmphead.bV5Height * 2];   // overcommit ftw
    if (tempgfx == NULL) return 1;
    if (bmp_load8_data(bitmapFile, &bmphead, tempgfx, NULL, -1)) {
        return 1;
    }

    info->atlas_width  = bmphead.bV5Width;
    info->atlas_height = bmphead.bV5Height;

    // generate bit mask for font
    info->atlasData = new uint32_t[(info->atlas_width * info->atlas_height)>>1];
    if (info->atlasData == NULL) return 1;

    uint32_t *dst = info->atlasData;
    uint8_t  *src = tempgfx;
    font_generate_mask(dst, src, info->atlas_width*info->atlas_height>>2);
    

    info->atlas_width  = (info->atlas_width >> 2) << 1; 
    info->start_char   = startChar;

    // attach glyph descriptor data
    info->desc = desc;

    return 0;
}

int font_free(font_info_t *info) {
    if (info->atlasData != NULL) {
        delete[] info->atlasData;
        info->atlasData = NULL;
    }
    return 0;
}

extern "C" void draw_glyph_masked_a(uint8_t *dst, uint32_t *src, int w, int h, int dst_fixup, int src_fixup);
#pragma aux draw_glyph_masked_a parm [edi] [esi] [ebx] [ecx] [edx] [eax] modify [eax ebx ecx edx esi edi]
extern "C" void draw_glyph_masked_color_a(uint8_t *dst, uint32_t *src, int w, int h, uint32_t colormask, int dst_fixup, int src_fixup);
#pragma aux draw_glyph_masked_color_a parm caller [edi] [esi] [ebx] [ecx] [edx] modify [eax ebx ecx edx esi edi]
extern "C" void draw_glyph_add_color_a(uint8_t *dst, uint32_t *src, int w, int h, uint32_t colormask, int dst_fixup, int src_fixup);
#pragma aux draw_glyph_add_color_a parm caller [edi] [esi] [ebx] [ecx] [edx] modify [eax ebx ecx edx esi edi]


// returns next x
int font_draw_char(uint8_t *dst, struct font_info_t *font, int ch, int x, int y, int dst_pitch, uint32_t colormask) {
    struct font_descriptor_t *desc = font->desc + ch - font->start_char;
    uint8_t *p = dst + (dst_pitch*y) + x;
    uint32_t *src = font->atlasData + desc->offset;
    if ((desc->width | desc->height) != 0) draw_glyph_masked_color_a(
        p - (dst_pitch*desc->yofs) + desc->xofs, 
        src,
        desc->width, desc->height, colormask,
        dst_pitch - (desc->width<<2),
        (font->atlas_width - (desc->width<<1)<<2)
    );
    return desc->advance;
}

int font_draw_string(uint8_t *dst, struct font_info_t *font, const char *str, int x, int y, int dst_pitch, uint32_t colormask) {
    if((font == NULL) || (str == NULL)) return 0;
    uint8_t *p = dst + (dst_pitch*y) + x;

    while (*str != 0) {
        char ch = *str - font->start_char;
        struct font_descriptor_t *desc = font->desc + ch;
        uint32_t *src = font->atlasData + desc->offset;
        if ((desc->width | desc->height) != 0) draw_glyph_masked_color_a(
            p - (dst_pitch*desc->yofs) + desc->xofs, 
            src,
            desc->width, desc->height, colormask,
            dst_pitch - (desc->width<<2),
            (font->atlas_width - (desc->width<<1)<<2)
        );
        p += desc->advance;
        str++;
    }
    return (p - dst);
}

// returns next x
int font_draw_char_add(uint8_t *dst, struct font_info_t *font, int ch, int x, int y, int dst_pitch, uint32_t colormask) {
    struct font_descriptor_t *desc = font->desc + ch - font->start_char;
    uint8_t *p = dst + (dst_pitch*y) + x;
    uint32_t *src = font->atlasData + desc->offset;
    if ((desc->width | desc->height) != 0) draw_glyph_add_color_a(
        p - (dst_pitch*desc->yofs) + desc->xofs, 
        src,
        desc->width, desc->height, colormask,
        dst_pitch - (desc->width<<2),
        (font->atlas_width - (desc->width<<1)<<2)
    );
    return desc->advance;
}

int font_draw_string_add(uint8_t *dst, struct font_info_t *font, const char *str, int x, int y, int dst_pitch, uint32_t colormask) {
    if((font == NULL) || (str == NULL)) return 0;
    uint8_t *p = dst + (dst_pitch*y) + x;

    while (*str != 0) {
        char ch = *str - font->start_char;
        struct font_descriptor_t *desc = font->desc + ch;
        uint32_t *src = font->atlasData + desc->offset;
        if ((desc->width | desc->height) != 0) draw_glyph_add_color_a(
            p - (dst_pitch*desc->yofs) + desc->xofs, 
            src,
            desc->width, desc->height, colormask,
            dst_pitch - (desc->width<<2),
            (font->atlas_width - (desc->width<<1)<<2)
        );
        p += desc->advance;
        str++;
    }
    return (p - dst);
}

void scroller_init(scroller_t *scroll) {
    scroll->textofs = 0;
    scroll->pixelpos = 0;
    scroll->glyphpos = 0;
    scroll->length  = strlen(scroll->text);
    scroll->curtext = scroll->text;
}

#if 1
void scroller_advance(scroller_t *scroll, int dx) {
    bool run = true;
    const char *str = scroll->curtext;
    int next_pixelpos = scroll->pixelpos + dx;
    scroll->glyphpos -= dx;

    while (run) {
        char ch = *str - scroll->font->start_char;
        struct font_descriptor_t *desc = scroll->font->desc + ch;
        if (scroll->glyphpos <= -desc->advance) {
            scroll->glyphpos += desc->advance;
            str++; if (*str == 0) str = scroll->text;
        } else {
            run = false;
        }
    }

    scroll->curtext = str;
}
#endif

// draw scroller
void scroller_draw(scroller_t *scroll, uint8_t *dst, int x, int y, int width, int dst_pitch, uint32_t color) {
    if (scroll == 0) return;
    
    x += scroll->glyphpos;
    uint8_t *p = dst + (dst_pitch*y) + (x);
    const char *str = scroll->curtext;
    font_info_t *font = scroll->font;

    while ((x < width) && (*str != 0)) {
        char ch = *str - font->start_char;
        struct font_descriptor_t *desc = font->desc + ch;
         uint32_t *src = font->atlasData + desc->offset;
        if ((desc->width | desc->height) != 0) draw_glyph_masked_color_a(
            p - (dst_pitch*desc->yofs) + desc->xofs, 
            src,
            desc->width, desc->height, color,
            dst_pitch - (desc->width<<2),
            (font->atlas_width - (desc->width<<1)<<2)
        );
        p += desc->advance;
        x += desc->advance;
        str++;
    }
}
