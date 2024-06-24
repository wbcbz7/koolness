#pragma once
#include <stdint.h>
#include "vec.h"

enum {
    LINE_STYLE_MOV = 0,
    LINE_STYLE_AVG,
    LINE_STYLE_ADD,
    LINE_STYLE_SUB,
    LINE_STYLE_ADDS,
    LINE_STYLE_SUBS,
    LINE_STYLE_ADD_AA,
    LINE_STYLE_SUB_AA,
    LINE_STYLE_ADDS_AA,
    LINE_STYLE_SUBS_AA,
    LINE_STYLE_ADDS_128,            // saturated on start point only
    LINE_STYLE_ADDS_128_AA,         // saturated on start point only
};

void linedraw_bresenham(uint8_t *dst, int pitch, vec2i *a, vec2i *b, int color);
void linedraw_subpixel(uint8_t *dst, int pitch, vec2x *a, vec2x *b, int color, int style);
int lineclip(vec2i *a, vec2i *b, int xres, int yres);

// linedraw setup
void linedraw_calc_aa_table(int color, int invert = 0);
void linedraw_calc_pitch_table(int pitch);
