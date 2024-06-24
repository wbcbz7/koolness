#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "flexptc.h"
#include "palerp.h"

#include "vec.h"
#include "fast_obj.h"
#include "polydraw.h"
#include "linedraw.h"
#include "matrix.h"
#include "3d.h"
#include "mesh.h"
#include "3dclip.h"
#include "3dsort.h"
#include "bmpload.h"
#include "rectdraw.h"

#include "esfmout.h"
#include "irq0.h"
#include "opmplay.h"
#include "player.h"
#include "fonts.h"

#include "main.h"
#include "introrc.h"

namespace intro_resources {

ptc_surface *alpha_overlay;
ptc_surface *dot_overlay;

int init() {
    // load alpha overlay
    BITMAPV5HEADER bmphead;
    if (bmp_load8_header("assets\\alpha.bmp", &bmphead) != 0) {
        printf("can't load texture!\n");
        return 1;
    }
    alpha_overlay = new ptc_surface;
    if (alpha_overlay == NULL) {
        printf("can't allocate memory!\n");
        return 1;
    }
    alpha_overlay->create(bmphead.bV5Width, bmphead.bV5Height, PTC_SURFACE_INDEX8, 1, 0);
    if (bmp_load8_data("assets\\alpha.bmp", &bmphead, (uint8_t*)alpha_overlay->data, NULL, -1)) {
        printf("can't load texture!\n");
        return 1;
    }

    // load trendwhore overlay
    if (bmp_load8_header("assets\\overlay.bmp", &bmphead) != 0) {
        printf("can't load texture!\n");
        return 1;
    }
    dot_overlay = new ptc_surface;
    if (dot_overlay == NULL) {
        printf("can't allocate memory!\n");
        return 1;
    }
    dot_overlay->create(bmphead.bV5Width, bmphead.bV5Height, PTC_SURFACE_INDEX8, 1, 0);
    if (bmp_load8_data("assets\\overlay.bmp", &bmphead, (uint8_t*)dot_overlay->data, NULL, -1)) {
        printf("can't load texture!\n");
        return 1;
    }

    return 0;
}

void done() {
    delete alpha_overlay;
    delete dot_overlay;
}
}


namespace common_resources {

// common resources here
ptc_surface *rec_sprite;
uint8_t     *overgrid;

// fonts
font_info_t bigfont;
font_info_t smallfont;

int init() {
    // load alpha overlay
    BITMAPV5HEADER bmphead;
    if (bmp_load8_header("assets\\rec.bmp", &bmphead) != 0) {
        printf("can't load texture!\n");
        return 1;
    }
    rec_sprite = new ptc_surface;
    if (rec_sprite == NULL) {
        printf("can't allocate memory!\n");
        return 1;
    }
    rec_sprite->create(bmphead.bV5Width, bmphead.bV5Height, PTC_SURFACE_INDEX8, 1, 0);
    if (bmp_load8_data("assets\\rec.bmp", &bmphead, (uint8_t*)rec_sprite->data, NULL, -1)) {
        printf("can't load texture!\n");
        return 1;
    }

    overgrid = new uint8_t[(X_RES*Y_RES)/(OVR_GRID_SIZE*OVR_GRID_SIZE) + 256]; // cause we're lame
    if (overgrid == NULL) {
        printf("can't allocate memory!\n");
        return 1;
    }

    // init fonts
    font_load(&bigfont, bigfont_desc, "assets\\bigfont.bmp", 32);
    font_load(&smallfont, smallfont_desc, "assets\\smalfont.bmp", 32);

    return 0;
}

void done() {
    delete rec_sprite;
    delete overgrid;
}

}

// draw sprite grid
int spritegrid_draw(ptc_surface *dst, ptc_surface *src, uint8_t *grid, uint32_t gridsize, uint32_t xgrid, uint32_t ygrid) {
    uint8_t  *gr = grid;
    uint8_t *s = ((uint8_t*)src->lockrect());
    uint8_t *d = ((uint8_t*)dst->lockrect());
    uint32_t dstpitch = dst->pitch, gridpitch = gridsize;
    uint32_t charsize = gridsize*gridsize;

    for (int yy = 0; yy < ygrid; yy++) {
        for (int xx = 0; xx < xgrid; xx++) {
            if (*gr > 0)
                rect_blit_add(d, s + charsize*(*gr-1), gridsize>>2, gridsize, (dstpitch - gridsize), 0);
            d += gridsize;
            gr++;
        }
        d += (OVR_GRID_SIZE * dst->pitch) - dst->width;
    }

    return 0;
}

// xorshift seed global storage
unsigned long _xorshift_seed_;

static int myseed = 0;
uint32_t myrand() {
    return (myseed = xorshift32(myseed));
}

int overlay_trendwhore(float t, uint8_t *grid, uint32_t xgrid, uint32_t ygrid,
    int overlayType, int overlayMax, int overlayFreq, int overlayCount) {
    // clear
    memset(grid, 0, xgrid * ygrid);

#if 0
    int overlayType = rocket->getTrack("overlay#type", t);
    int overlayMax = rocket->getTrack("overlay#max", t);
    int overlayFreq = rocket->getTrack("overlay#freq", t);
    int overlayCount = rocket->getTrack("overlay#count", t);
#endif
    int it = t * overlayFreq; myseed = it;
    switch (overlayType) {
        case 1:
            // dots only
            for (int i = 0; i < overlayCount; i++) {
                int xx = myrand() % xgrid, yy = myrand() % ygrid;
                int xe = xx + (myrand() % (xgrid - xx)), ye = yy + (myrand() % (ygrid - yy));

                uint8_t *p = grid + (xgrid * yy);
                for (int y = yy; y < ye; y++) {
                    for (int x = xx; x < xe; x++) {
                        *(p + x) = (myrand() % (overlayMax + 1)) + 1;
                    }
                    p += xgrid;
                }
            }
            break;

        case 3:
            // random
            for (int i = 0; i < xgrid*ygrid; i++) grid[i] = (myrand() & 1) + 1;
            break;

        default: 
        break;
    }
    return 0;
}


void dirblur(uint8_t* p, int len, int disp) {
    uint8_t *v = &p[disp];
    uint32_t   l  = (len - 4*disp),
             _edx = -4*disp;               

    _asm {
        mov     esi, p
        mov     ecx, l
        mov     edi, v
        mov     edx, _edx
        push    ebp
        
        _loop:
        mov     eax, [edi]
        add     esi, 4
        
        mov     ebp, [edi + edx]
        and     eax, 0xFEFEFEFE
        
        shr     eax, 1
        and     ebp, 0xFEFEFEFE
        
        shr     ebp, 1
        add     edi, 4
        
        add     eax, ebp
        sub     ecx, 2
        
        mov     [edi + edx - 4], eax
        jnz     _loop

        pop     ebp
    }
}

void glitch(ptc_surface *surf, glist_t *gl, int vel, int size) {
    if (vel > 0) {
        glist_t *p = gl;
        int randnum = (rand() % (size + 1))+1;
        int randofs = (rand() % (size + 1));
        for (int i = 0; i < Y_RES; i++) if ((i != 0) && (((i + randofs) % randnum) != 0)) {*p = *(p-1); p++;} else {
            randnum = (rand() % (size + 1))+1;
            p->active = rand() & 1;
            p->disp   = (rand() % (vel)) & ~1; // four pixels boundary
            p++;
        }
        uint8_t* pd = (uint8_t*)surf->data + surf->pitch;
        p = gl;
        glist_t de;
        memcpy(&de, gl, sizeof(glist_t));
        for (int k = 1; k < Y_RES-1; k++) {
            if (p->active) {memcpy(&de, p, sizeof(glist_t));}
            dirblur(pd, surf->width, de.disp);
            p++; pd += surf->pitch;
        }
    }
}

void interlace(ptc_surface *surf) {
    uint8_t *dst = (uint8_t*)surf->data + surf->pitch;
    uint32_t fixup = 2*surf->pitch - surf->width;
    uint32_t w = surf->width/8, h = surf->height/2;
    _asm {
        mov     edi, [dst]
        mov     edx, [fixup]
        mov     esi, [w]
        mov     ecx, [h]
        _y_loop:
        push    ecx
        mov     ecx, esi
        _x_loop:
        mov     eax, [edi + 0]
        mov     ebx, [edi + 4]
        shr     eax, 1
        and     ebx, 0xFEFEFEFE
        shr     ebx, 1
        and     eax, 0x7F7F7F7F
        mov     [edi + 4], ebx
        add     edi, 8
        mov     [edi - 8], eax
        dec     ecx
        jnz     _x_loop

        pop     ecx
        add     edi, edx
        dec     ecx
        jnz     _y_loop
    }
}

int do_overlay(ptc_surface *dst, glist_t* glist, int glvel, int glsize, float t) {
    // alpha overlay
    rect_blit_subs((uint8_t*)dst->data, (uint8_t*)intro_resources::alpha_overlay->data,
        X_RES>>2, Y_RES, dst->pitch - dst->width, 0
    );

    spritegrid_draw(dst, intro_resources::dot_overlay, common_resources::overgrid, OVR_GRID_SIZE, (X_RES / OVR_GRID_SIZE), (Y_RES / OVR_GRID_SIZE));

    // glitch it
    glitch(dst, glist, glvel, glsize);

    // interlace it
    interlace(dst);

    // draw record indicator
    if ((fmod(t, 0.6) < 0.3)) {
        rect_blit_add((uint8_t*)dst->data + dst->pitch*(12) + (X_RES-24),
            (uint8_t*)common_resources::rec_sprite->data, 8/4, 8, (dst->pitch - 8), 0);
    }

    // draw indent lines
    {
        uint8_t *p = (uint8_t*)dst->data;
        int x = 4, y = 4, w = 16, h = 1;
        rect_fill_add(p+(dst->pitch*y)+(x&~3), 0x18181818, w>>2, h, dst->pitch-w, 0);
        x = 4, y = 5, w = 4, h = 15;
        rect_fill_add(p+(dst->pitch*y)+(x&~3), 0x00000018, w>>2, h, dst->pitch-w, 0);

        x = X_RES-4-16, y = 4, w = 12, h = 1;
        rect_fill_add(p+(dst->pitch*y)+(x&~3), 0x18181818, w>>2, h, dst->pitch-w, 0);
        *(uint32_t*)(p+(dst->pitch*y)+X_RES-8) += 0x00181818;
        x = X_RES-8, y = 4, w = 4, h = 16;
        rect_fill_add(p+(dst->pitch*y)+(x&~3), 0x18000000, w>>2, h, dst->pitch-w, 0);

        
        x = X_RES-4-16, y = Y_RES-4, w = 16, h = 1;
        rect_fill_add(p+(dst->pitch*y)+(x&~3), 0x18181818, w>>2, h, dst->pitch-w, 0);
        x = X_RES-4, y = Y_RES-4-16, w = 4, h = 17;
        rect_fill_add(p+(dst->pitch*y)+(x&~3), 0x00000018, w>>2, h, dst->pitch-w, 0);

        x = 4, y = Y_RES-4, w = 16, h = 1;
        rect_fill_add(p+(dst->pitch*y)+(x&~3), 0x18181818, w>>2, h, dst->pitch-w, 0);
        x = 4, y = Y_RES-4-16, w = 4, h = 16;
        rect_fill_add(p+(dst->pitch*y)+(x&~3), 0x00000018, w>>2, h, dst->pitch-w, 0);
    }

    // draw
    {
        char text[16];
        snprintf(text, sizeof(text), "%d", player_ctx.opm.pos.frame);
        int len = font_get_length(&common_resources::smallfont, text);
        font_draw_string((uint8_t*)dst->data, &common_resources::smallfont, text, X_RES-len-8, Y_RES-8, dst->pitch, 0x20202020);  
    }

    return 0;
}

void drawflare(ptc_surface &dst, uint8_t *src, vec2i &f, int ures, int vres, uint32_t colormask) {
    
    static int x_adj, u_adj, fsize, fsize2, spr_index;
    
    signed int px, py;
    
    px = f.x-(ures>>1); py = f.y-(vres>>1);
    
    // dirty way of clipping :)
    if ((px <= 0) || (px >= X_RES-ures-2) || (py <= 0) || (py >= Y_RES-vres-2)) {
        return;
    }
    
    // set static vars
    fsize = vres; fsize2 = (ures >> 2); u_adj = 0; x_adj = dst.pitch - ures;
    spr_index = (px & 3);
    
    px &= ~3; // small fixup
        
    src += (ures*(vres*spr_index));
    
    uint8_t *dstptr = ((uint8_t*)dst.lock() + ((py) * dst.pitch) + ((px) << 0));
    
     rect_blit_add(
        dstptr,
        src, 
        fsize2,
        fsize,
        x_adj,
        u_adj
    );
}    

