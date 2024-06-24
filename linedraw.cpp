#include <stdint.h>
#include "vec.h"
#include "linedraw.h"

// line renderers
#pragma aux __linedrawcall "*_" parm caller [edi] [ebx] [esi] [ecx] [edx] [eax] \
                        value [eax] modify [eax ebx ecx edx esi edi]

#define __linedrawcall __declspec(__pragma("__linedrawcall"))

//                                               edi        ebx      esi      ecx       edx        eax
//                                              dst+x       y        dydx    length    pitch      color
typedef void __linedrawcall (*linedraw_proc_t) (uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);


// linedraw antialiasing table
extern "C" {
    uint8_t  linedraw_aa_tab[16*2];
    uint32_t linedraw_pitchtab[256];

    // asm routines
    void __linedrawcall linedraw_mov_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_avg_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_add_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_sub_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_adds_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_subs_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_add_aa_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_adds_aa_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_subs_aa_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);

    void __linedrawcall linedraw_adds_128_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_adds_128_aa_dx_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);

    void __linedrawcall linedraw_mov_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_avg_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_add_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_sub_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_adds_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_subs_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_add_aa_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_adds_aa_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_subs_aa_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);

    void __linedrawcall linedraw_adds_128_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    void __linedrawcall linedraw_adds_128_aa_dy_a(uint8_t*, int32_t, uint32_t, uint32_t, uint32_t, uint32_t);
}

// line type tables
static linedraw_proc_t linedraw_proc[32] = {
    // dx
    linedraw_mov_dx_a, linedraw_avg_dx_a,
    linedraw_add_dx_a, linedraw_sub_dx_a,
    linedraw_adds_dx_a, linedraw_subs_dx_a,
    linedraw_add_aa_dx_a, linedraw_add_aa_dx_a,
    linedraw_adds_aa_dx_a, linedraw_subs_aa_dx_a,
    linedraw_adds_128_dx_a, linedraw_adds_128_aa_dx_a, 0, 0, 0, 0,

    // dy
    linedraw_mov_dy_a, linedraw_avg_dy_a,
    linedraw_adds_dy_a, linedraw_subs_dy_a,
    linedraw_adds_aa_dy_a, linedraw_subs_dy_a,
    linedraw_add_aa_dy_a, linedraw_add_aa_dy_a,
    linedraw_adds_aa_dy_a, linedraw_subs_aa_dy_a,
    linedraw_adds_128_dy_a, linedraw_adds_128_aa_dy_a
};

// -------------------------

void linedraw_calc_aa_table(int color, int invert) {
    uint8_t *p = linedraw_aa_tab;
    
    int c = 0, dc = (color << 4);
    for (int i = 0; i < 16; i++) {
        if (invert) {
            *(p+0)   = -(color - (c >> 8));
            *(p+16) = -(c >> 8);
        } else {
            *(p+0)   = color - (c >> 8);
            *(p+16) = (c >> 8);
        }
        c += dc; 
        p++;
    }
}

void linedraw_calc_pitch_table(int pitch) {
    uint32_t *p = linedraw_pitchtab;
    uint32_t a = 0;
    for (int i = 0; i < 256; i++) {
        *p++ = a; a += pitch;
    }
}

// not a bresenham :]
void linedraw_subpixel(uint8_t *dst, int pitch, vec2x *a, vec2x *b, int color, int style) {  
    // get dx
    long dx = abs(ceilx(b->x) - ceilx(a->x));
    long dy = abs(ceilx(b->y) - ceilx(a->y));

    if (dx > dy) {
        // 0 < |tan(a)| < 1

        if (a->x > b->x) {
            vec2x *tmp = a; a = b; b = tmp;
        }
        long dydx;
        if (dx == 0) return; else
        if (dx == 1) {
            dydx = (0x10000 << 14) / (b->x - a->x);
            dydx = imul14(dydx, (b->y - a->y));
        } else {
            dydx = idiv16((b->y - a->y), (b->x - a->x));
        }
        // prestep
        long y = a->y + imul16(((ceilx(a->x) << 16) - a->x), dydx);
        // draw line for x1 to (x2-1) (!!!!!!)
        linedraw_proc_t proc = linedraw_proc[16 + style];
        proc(dst + ceilx(a->x), y, dydx, (ceilx(b->x) - ceilx(a->x)), linedraw_pitchtab[1], color);
    } else {
        // |tan(a)| >= 1

        if (a->y > b->y) {
            vec2x *tmp = a; a = b; b = tmp;
        }
        long dxdy;
        if (dy == 0) return; else
        if (dy == 1) {
            dxdy = (0x10000 << 14) / (b->y - a->y);
            dxdy = imul14(dxdy, (b->x - a->x));
        } else {
            dxdy = idiv16((b->x - a->x), (b->y - a->y));
        }
        // prestep
        long x = a->x + imul16(((ceilx(a->y) << 16) - a->y), dxdy);
        // draw line for y1 to (y2-1) (!!!!!!)
        linedraw_proc_t proc = linedraw_proc[0 + style];
        proc(dst + linedraw_pitchtab[ceilx(a->y)], x, dxdy, (ceilx(b->y) - ceilx(a->y)), linedraw_pitchtab[1], color);
    }  
};

#define vcode(p) (((p->x < 0) ? 1 : 0) | ((p->x >= xres) ? 2 : 0) | ((p->y < 0) ? 4 : 0) | ((p->y >= yres) ? 8 : 0))    
 
int lineclip(vec2i *a, vec2i *b, int xres, int yres) {
    int code_a, code_b, code;
    vec2i *c;
 
    code_a = vcode(a);
    code_b = vcode(b);
 
    while (code_a || code_b) {
        if (code_a & code_b)
            return -1;
 
        if (code_a) {
            code = code_a;
            c = a;
        } else {
            code = code_b;
            c = b;
        }
 
        if (code & 1) {
            c->y += (a->y - b->y) * (0 - c->x) / (a->x - b->x + ee);
            c->x = 0;
        } else if (code & 2) {
            c->y += (a->y - b->y) * (xres - c->x) / (a->x - b->x + ee);
            c->x = xres - 1;
        }

        if (code & 4) {
            c->x += (a->x - b->x) * (0 - c->y) / (a->y - b->y + ee);
            c->y = 0;
        } else if (code & 8) {
            c->x += (a->x - b->x) * (yres - c->y) / (a->y - b->y + ee);
            c->y = yres - 1;
        }
 
        if (code == code_a)
            code_a = vcode(a);
        else
            code_b = vcode(b);
    }
 
    return 0;
}

void linedraw_bresenham(uint8_t *dst, int pitch, vec2i *a, vec2i *b, int color) {
    if (a->x < b->x) {
        vec2i *tmp = a; a = b; b = tmp;
    }

    int delta_x, delta_y, dx, dy, t, d;
    int xerr = 0, yerr = 0;
    
    int sx, sy;
    long col = color;
    // determine dx and dy
    delta_x = b->x - a->x;
    delta_y = b->y - a->y;
    // determine steps by x and y axes (it will be +1 if we move in forward
    // direction and -1 if we move in backward direction
    if (delta_x > 0) dx = 1; else if (delta_x == 0) dx = 0; else dx = -1;
    if (delta_y > 0) dy = 1; else if (delta_y == 0) dy = 0; else dy = -1;
    delta_x = abs(delta_x);
    delta_y = abs(delta_y);
    // select largest from deltas and use it as a main distance
    if (delta_x > delta_y) d = delta_x; else d = delta_y;
    
    sx = a->x; sy = a->y;
    uint8_t *p = dst + (sy * pitch) + sx;
    for (t = 0; t <= d; t++)	{	
        *p -= color;
        
        // increasing error
        xerr += delta_x;
        yerr += delta_y;
        // if error is too big then we should decrease it by changing
        // coordinates of the next plotting point to make it closer
        // to the true line
        if (xerr > d) {	
            xerr -= d;
            p += dx;
        }	
        if (yerr > d) {	
            yerr -= d;
            p += (dy * pitch);
        }	
    }
}

#if 0
// nop
void __linedrawcall linedraw_nop(uint8_t* dst, int32_t x, uint32_t dxdy, uint32_t length, uint32_t pitch, uint32_t color) {
}

// move, non-aa, dx
void __linedrawcall linedraw_mov_dx(uint8_t* dst, int32_t x, uint32_t dxdy, uint32_t length, uint32_t pitch, uint32_t color) {
    do {
        *(dst + (x >> 16) + 0) = color;
        x += dxdy; dst += pitch;
    } while (--length);
}

// move, non-aa, dy
void __linedrawcall linedraw_mov_dy(uint8_t* dst, int32_t y, uint32_t dydx, uint32_t length, uint32_t pitch, uint32_t color) {
    do {
        *(dst + linedraw_pitchtab[(y >> 16)] + 0) = color;
        y += dydx; dst++;
    } while (--length);
}

// saturated sub, non-aa, dx
void __linedrawcall linedraw_subs_dx(uint8_t* dst, int32_t x, uint32_t dxdy, uint32_t length, uint32_t pitch, uint32_t color) {
    do {
        int a = *(dst + (x >> 16) + 0) - color;
        if (a < 0) a = 0;
        *(dst + (x >> 16) + 0) = a;
        x += dxdy; dst += pitch;
    } while (--length);
}

// saturated sub, non-aa, dy
void __linedrawcall linedraw_subs_dy(uint8_t* dst, int32_t y, uint32_t dydx, uint32_t length, uint32_t pitch, uint32_t color) {
    do {
        int a = *(dst + linedraw_pitchtab[(y >> 16)] + 0) - color;
        if (a < 0) a = 0;
        *(dst + linedraw_pitchtab[(y >> 16)] + 0) = a;
        y += dydx; dst++;
    } while (--length);
}

// saturated add, non-aa, dx
void __linedrawcall linedraw_adds_dx(uint8_t* dst, int32_t x, uint32_t dxdy, uint32_t length, uint32_t pitch, uint32_t color) {
    do {
        int a = *(dst + (x >> 16) + 0) + color;
        if (a > 255) a = 255;
        *(dst + (x >> 16) + 0) = a;
        x += dxdy; dst += pitch;
    } while (--length);
}

// saturated add, non-aa, dy
void __linedrawcall linedraw_adds_dy(uint8_t* dst, int32_t y, uint32_t dydx, uint32_t length, uint32_t pitch, uint32_t color) {
    do {
        int a = *(dst + linedraw_pitchtab[(y >> 16)] + 0) + color;
        if (a > 255) a = 255;
        *(dst + linedraw_pitchtab[(y >> 16)] + 0) = a;
        y += dydx; dst++;
    } while (--length);
}
#endif