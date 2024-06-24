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
#include "introrc.h"
#include "main.h"

namespace menu {

#define X_PITCH 384
const float fov = 120.0;

int to_fadeout = 0; 

bool show_cpu_time = false;
bool show_debug_info = false;

// --------------------
// keyboard UI
void handle_keyboard() {
    int volume;

    // keyboard processing
    if (kbhit()) {
        int ch = getch(); if (ch == 0) ch |= (getch() << 8);
        switch(ch) {
            case 0x4800: case 'Q': case 'q':
                player_ctx.selector.cursor--; if (player_ctx.selector.cursor < 0) player_ctx.selector.cursor = 0;
                break;
            case 0x5000: case 'A': case 'a':
                player_ctx.selector.cursor++; if (player_ctx.selector.cursor >= arrayof(modulelist)) player_ctx.selector.cursor = arrayof(modulelist)-1;
                break;
            case '-' :
                player_ctx.volume = esfm_get_volume()-1; if (player_ctx.volume < 0) player_ctx.volume = 0;
                esfm_set_volume(player_ctx.volume);
                break;
            case '+': case '=': 
                player_ctx.volume = esfm_get_volume()+1; if (player_ctx.volume > 15) player_ctx.volume = 15;
                esfm_set_volume(player_ctx.volume);
                break;
            case 'D' : case 'd':
                dos_shell();
                break;
            case 'R' : case 'r':
                show_cpu_time = !show_cpu_time;
                break;
            case 'P' : case 'p':
                mainprops.lq_mode = !mainprops.lq_mode;
                break;
            case 'L' : case 'l':
                player_ctx.playmode = (player_ctx.playmode + 1) % PLAY_MODE_COUNT;
                break;
            case 'F' : case 'f':
                mainprops.fast_esfm_out = !mainprops.fast_esfm_out;
                break;
            case 'B' : case 'b':
                show_debug_info = !show_debug_info;
                break;
            case 10:
            case 13:
                // load selected module
                // TODO: 1) fadeout 2) delay :)
                //player_load_module(&player_ctx, modulelist[player_ctx.selector.cursor].file);
                //player_load_module_idx(&player_ctx, player_ctx.selector.cursor);
                if (player_ctx.selector.cursor != player_ctx.selector.sel) {
                    player_ctx.selector.next_sel = player_ctx.selector.cursor;
                    player_ctx.nextsong.volume = player_ctx.volume;
                    player_ctx.nextsong.timeout = 64;
                    player_ctx.nextsong.timeout_mask = 3;
                    player_ctx.nextsong.run = true;
                }
                break;
            case 27: 
                to_fadeout++;
                break;
            default: break;
        }
    }
}

// --------------------------
#ifndef WIN32
void print_debug(uint8_t *ptr, uint32_t pitch, float dt) {
    uint32_t dpmi_mem_info[32];
    _asm {
        lea     edi, dpmi_mem_info
        mov     eax, 0x500
        int     0x31
    };
    uint16_t lowmem;
    _asm {
        mov     ebx, -1
        mov     eax, 0x4800
        int     0x21
        mov     [lowmem], bx
    }
    char buf[128];
    snprintf(buf, sizeof(buf), "free memory: dpmi %d kb, low %d kb | timer mode: %s",
        dpmi_mem_info[0] >> 10, lowmem >> 6,
        mainprops.use_irq0 ? "IRQ0" : "RTC"
    );
    font_draw_string(ptr, &common_resources::smallfont, buf, 0, 170, pitch);
    snprintf(buf, sizeof(buf), "frame time: %.2f ms, ESFM delays: %s, lc %d", dt*1000,
        mainprops.fast_esfm_out ? "no" : "yes",
        player_ctx.nextsong.loop_count
    );
    font_draw_string(ptr, &common_resources::smallfont, buf, 0, 179, pitch);
}
#else
#define print_debug(...)
#endif

// --------------------------------
// global draw list, z-sorted
std::vector<facelist_t> mesh_flist;
std::vector<zdata_t>    mesh_zmap;

// xstart, xend
int vu_bars[72][2];
uint8_t  *logo_bitmap;
uint32_t *logo_bitmap_masked;

void gen_bars() {
    int  idx = 0;
    int* bar = player_ctx.bars;
    int* k = player_ctx.key;
    bar_keyofs_t *bar_keyofs = player_ctx.bars_keyofs;

    for (int ch = 0; ch < 18; ch++) {
        for (int op = 0; op < 4; op++) {
            // add sinus for bars offset
            int yy = ((player_ctx.bars_offset*2) * (rand() & 15)) >> 4;
            if (bar_keyofs->value > 0) yy += (bar_keyofs->value >> 2);

            // get height
            int len = *bar + bar_keyofs->value - 12;
            if (len < 0)  len = 0;
            len -= (len >> 3);

            vu_bars[idx][0] = yy;
            vu_bars[idx][1] = len;

            bar++;
            idx++;
        }
        bar_keyofs++;
        k++;
    }
}

// final object
mesh_t mesh;

#include "scroll.h"

scroller_t scroller_desc = {
    &common_resources::bigfont,
    scroll_text
};

uint8_t  playmode_sprite[8*24];

ptc_palette pal(256, 32);
ptc_surface surf;

uint8_t  texture[256*256*2];

// initialization here
int init() {
    surf.create(X_RES, Y_RES, X_PITCH, PTC_SURFACE_INDEX8, 1, 0);

    // init palette
    argb32 pal0, pal1;
    //pal0.v = 0x002E49;
    pal0.v = 0x001020;
    pal1.v = 0xC1F4FF;
    vec3f cpow = {1,0.8,0.75};
    //pal_lerp((argb32*)pal.data, &pal0, &pal1, 1, 127);
    pal_calc(&pal, pal0, pal1, cpow, 0, 128, false);
    // make overflow entries 128-207
    for (int i = 128; i < 208; i++) pal.data[i] = pal.data[127];

    // TODO: logo palette here!!

    pal.data[253].val = 0x040404;
    pal.data[254].val = 0xD0E0F0;
    pal.data[255].val = 0xFFFFFF;

    mesh_load(mesh, "assets\\ico320.obj");
    mesh.style = FACE_POLY | POLY_STYLE_DOUBLESIDED | POLY_STYLE_MOV | POLY_STYLE_WIREFRAME;
    printf("a"); fflush(stdout);

#if 1
    for (int i = 0; i < mesh.p.size(); i++) {
        float phix = atan2(mesh.p[i].y, mesh.p[i].z);
        float phiy = atan2(mesh.p[i].x, mesh.p[i].z);
        float phiz = atan2(mesh.p[i].y, mesh.p[i].x);

        // warp meshes
        mesh.p[i].x *= (1.0 + 0.3*sin(1.5*phix));
        mesh.p[i].y *= (1.0 + 0.3*sin(1*phiy));
        mesh.p[i].z *= (1.0 + 0.4*sin(1.5*phiz));

        //mesh_p[i].y *= (1.0 + 0.4*sin(pi*mesh_p[i].y)*cos(pi*mesh_p[i].z));
        //mesh_p[i].z *= (1.0 + 0.4*sin(pi*mesh_p[i].z)*cos(pi*mesh_p[i].x));
    }
#endif

    mesh_calc_nomals(mesh);

    // init scroller
    scroller_init(&scroller_desc);

    // load playmode sprite
    BITMAPV5HEADER bmphead;
    if (bmp_load8_header("assets\\playmode.bmp", &bmphead) != 0) {
        printf("can't load texture!\n");
        return 1;
    }
    if (bmp_load8_data("assets\\playmode.bmp", &bmphead, playmode_sprite, NULL, -1)) {
        printf("can't load texture!\n");
        return 1;
    }
    // load texture
    if (bmp_load8_header("assets\\tex2.bmp", &bmphead) != 0) {
        printf("can't load texture!\n");
        return 1;
    }
    if (abs(bmphead.bV5Height) != 256 && bmphead.bV5Width != 256) {
        printf("dimensions must be 256x256!\n");
        return 1;
    }
    if (bmp_load8_data("assets\\tex2.bmp", &bmphead, texture, NULL, -1)) {
        printf("can't load texture!\n");
        return 1;
    }
    // preshift and copy overflow
    for (int i = 0; i < 256*256; i++) texture[i] >>= 1;
    memcpy(texture + 256*256, texture, 256*256);

    argb32 logopal[256];
    // load logo!
    if (bmp_load8_header("assets\\logo.bmp", &bmphead) != 0) {
        printf("can't load texture!\n");
        return 1;
    }
    logo_bitmap_masked = new uint32_t[((bmphead.bV5Height * bmphead.bV5Width) >> 2) << 1];
    logo_bitmap = new uint8_t[bmphead.bV5Height * bmphead.bV5Width]; // stop memory fragmentation
    if (bmp_load8_data("assets\\logo.bmp", &bmphead, logo_bitmap, (uint32_t*)logopal, -1)) {
        printf("can't load texture!\n");
        return 1;
    }
    font_generate_mask(logo_bitmap_masked, logo_bitmap, (bmphead.bV5Height * bmphead.bV5Width)>>2);
    for (int i = 224; i < 248; i++) pal.data[i].val = logopal[i].v;
    
    delete[] logo_bitmap;
    return 0;
}

void done() {

}

void draw_bars(uint8_t *screen, int frame, int x, int y, int count, uint32_t color) {
    int xx = (X_RES - (count << 2)) >> 1;
    int yy = Y_RES - 4;
    color = (color * 0x010101);

    for (int i = 0; i < 72; i++) {
        uint8_t *p = (screen + X_PITCH*(yy - vu_bars[i][0]) + xx);
        int y = vu_bars[i][1]>>1;
        if (y > 0) _asm {
            mov     esi, [p]
            mov     ecx, [y]
            mov     eax, [color]
            _loop:
            add     [esi], eax
            sub     esi, X_PITCH
            dec     ecx
            jnz     _loop
        }
        xx += 4;
    }
}

#if 1
uint16_t *movetab;
uint16_t *movetab_tiled;
#else
uint16_t movetab[X_RES*Y_RES];
uint16_t movetab_tiled[X_RES*Y_RES];
#endif
uint8_t  fixup_hole[32][32];

void make_movetab(uint16_t *tab) {
    movetab_tiled = new uint16_t[X_RES*Y_RES];
    movetab = new uint16_t[X_RES*Y_RES];

    tab = movetab;

    for (double y = -Y_RES/2; y < Y_RES/2; y+=2) {
        for (double x = -X_RES/2; x < X_RES/2; x++) {
            int u, v;
            double r, a, l;
            r = sqrt(x*x + y*y);
            if (r < 1) r = 1;
            a = atan2(y, x) - pi/2;
            u = (a * 256 / pi);
            //v = r;
            v = (4096. / r);
            *tab++ = (u&0xFF) + ((v&0xFF)<<8);
        }
    }

    // tile it
    uint16_t* dst = movetab_tiled;
    for (int yy = 0; yy < Y_RES/(2*4); yy++) {
        for (int xx = 0; xx < X_RES/(8); xx++) {
            uint16_t *srctile = movetab + (xx * 8) + (yy * (4 * X_RES));
            // copy tile
            int y = 4;
            do {
                memcpy(dst, srctile, 8*sizeof(uint16_t));
                srctile += X_RES; dst += 8;
            } while (--y);
        }
    }

    delete[] movetab;

    // fill fixup hole
    uint8_t *tex = fixup_hole[0];
    for (int y = -16; y < 16; y++) {
        for (int x = -16; x < 16; x++) {
            double r, v;
            r = pow((sqrt(x*x + y*y) / 16.0), 0.4) * 127.0;  
            if (r > 127) r = 127;      
            *tex++ = 127 - r;
        }
    }
}

#pragma aux twirl_draw_tiled8x8_laced parm caller [edi] [esi] [edx] [ebx] [ecx] modify [eax ebx ecx edx esi edi]
extern "C" {
    void __declspec(__pragma("twirl_draw_tiled8x8_laced")) twirl_draw_tiled8x8_laced(uint8_t *buf, uint8_t* tex, uint16_t *tab, uint32_t ofs, uint32_t width);
}

void draw_movetab(uint8_t* screen, int frame) {
    uint16_t *tab = movetab;
    uint16_t uv = ((frame >> 6) & 0xFF) + (((frame >> 3) & 0xFF) << 8);
#if 1
    twirl_draw_tiled8x8_laced(screen, texture, movetab_tiled, uv, 0);
#else
    for (int y = 0; y < Y_RES; y += 2) {
        for (int x = 0; x < X_RES; x++) {
            uint8_t pix = texture[(*tab++ + uv) & 0xFFFF];
            *screen           = pix;
            *(screen+X_PITCH) = pix >> 1;
            screen += 1;
        }
        screen += 2*X_PITCH - X_RES;
    }
#endif
    // draw fixup hole
    if (!mainprops.lq_mode) rect_blit_subs(screen + X_PITCH*((Y_RES-32)/2) + ((X_RES-32)/2), fixup_hole[0], 32/4, 32, (X_PITCH - 32), 0);
}

void draw_module_selector(uint8_t *dst, float t) {
    int xx = 20, yy = 56;
    char tempstr[256];

    int pos = 0;
    uint32_t selcolor = ((127 - (int)(12+16*sin(t*5.5))) & 255) * 0x01010101;

    for (int i = 0; i < arrayof(modulelist); i++) {
        font_draw_string(dst, &common_resources::smallfont, modulelist[i].name, xx, yy, X_PITCH, i == player_ctx.selector.sel ? selcolor : 0xFFFFFFFF);
        if (player_ctx.selector.cursor == i)
            font_draw_char(dst, &common_resources::smallfont, '>', xx - 9 + 1.5*sin(t*3.0), yy, X_PITCH);

        yy += 10;
    }
}

void run() {
    // switch to sequentual playback
    player_ctx.playmode = PLAY_MODE_SEQUENTIAL;
#ifndef WIN32
    player_ctx.volume = esfm_get_volume();
#else
    player_ctx.volume = 15;
#endif
    linedraw_calc_pitch_table(surf.pitch);

#ifdef WIN32
    ptc_update(surf, PTC_BLT_WAIT);
    ptc_flip(PTC_FLIP_WAIT);
#endif

    // init tmap
    tmap_init(surf.data, X_RES, Y_RES, surf.pitch);

    // init clipper
    clipPlane clipPlanes[5];
    vec4f clipper = setclipper(fov, X_RES-2, Y_RES-2);
    setupClipPlanes5(0.5, clipper, clipPlanes);
    mesh_setup_draw(X_RES, Y_RES, fov);

    make_movetab(movetab);

    int frame_counter = 0;
    double t = ptc_getfloat(), st = t, frt = t;
        
    float fadeout_step = 255;
    bool run_part = true;
    while (run_part) {
        double nt = ptc_getfloat(), ot = nt - t; t = nt;

        // player stuff
        player_update_view();
        gen_bars();
        handle_keyboard();

        // logo flashing!
        memcpy(surf.pal->data, pal.data, sizeof(argb32)*256);
        {
            int flash = 192+((32.0*sin(8.0*t))+32);
            argb32 fadecol; fadecol.v = 0xD0D0F0;
            pal_fade(surf.pal, surf.pal, fadecol, flash, 224, 23);
        }

        // generate pallette fade
        if (to_fadeout == 1) {
            fadeout_step -= ot*128; fadeout_step = clamp(fadeout_step, 0, 255);
            if (fadeout_step == 0) run_part = false;
            argb32 fadecol; fadecol.v = 0x000000;
            pal_fade(surf.pal, surf.pal, fadecol, fadeout_step, 0, 256);
            int esfmvol = player_ctx.volume - 15 + ((int)fadeout_step >> 4);
            if (esfmvol < 0) esfmvol = 0;
            esfm_set_volume(esfmvol);
        } else if (to_fadeout > 1) run_part = false; else 
        if (to_fadeout == 0) {
            if ((t - st) < 1.0) {
                int fade_step = clamp((t - st) * 255, 0, 255);
                argb32 fadecol; fadecol.v = 0xFFFFFF;
                pal_fade(surf.pal, surf.pal, fadecol, fade_step, 0, 256);
            }
        }
        if (show_cpu_time) {
            surf.pal->data[0].val = 0x808080;
        }


        // not needed - twirl renders entrire screen
        //ptc_memset(surf.data, 0, surf.pitch * surf.height, 0);

        mesh_flist.clear();
        mesh_zmap.clear();

        // draw everything here
        vec4f origin = {145, 5, 250};
        mat4 rot;   rot4(rot, t*0.9, t*0.95, t*0.3);
        float rotscale = 70.0;// * (1.0 + 0.01*player_ctx.bars_offset);
        mat4 scale; scale4(scale, rotscale, rotscale, rotscale, 0.0);
        mat4 mt;    matmul4(mt, scale, rot); ofs4_imm(mt, origin.x, origin.y, origin.z);

        // transform mesh
        mesh_transform(mesh, mt);

        // project vertices
        mesh_project(mesh, fov, X_RES, Y_RES);

        // draw twirl mapper
        draw_movetab((uint8_t*)surf.data, t*1024);

        // draw vu meter bats
        draw_bars((uint8_t*)surf.data, t*1024, 0, 0, 72, 16);

        // draw indent lines
        {
            uint8_t *p = (uint8_t*)surf.data;
            int x = 4, y = 4, w = 16, h = 1;
            rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x18181818, w>>2, h, X_PITCH-w, 0);
            x = 4, y = 5, w = 4, h = 15;
            rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x00000018, w>>2, h, X_PITCH-w, 0);

            x = X_RES-4-16, y = 4, w = 12, h = 1;
            rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x18181818, w>>2, h, X_PITCH-w, 0);
            *(uint32_t*)(p+(X_PITCH*y)+X_RES-8) += 0x00181818;
            x = X_RES-8, y = 4, w = 4, h = 16;
            rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x18000000, w>>2, h, X_PITCH-w, 0);

            // selector / time count idents
            x = 152; y = 142; w = 4 ; h = 13;
            rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x00001000, w>>2, h, X_PITCH-w, 0);
            *(uint32_t*)(p+(X_PITCH*142)+152) += 0x00000010;
            x = 152-12; y = 142; w = 12 ; h = 1;
            rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x10101010, w>>2, h, X_PITCH-w, 0);


            x = 12; y = 148; w = 4 ; h = 13;
            rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x0A000000, w>>2, h, X_PITCH-w, 0);
            //*(uint32_t*)(p+(X_PITCH*160)+12) += 0x10000000;
            x = 16; y = 160; w = 12 ; h = 1;
            rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x0A0A0A0A, w>>2, h, X_PITCH-w, 0);
            //x = 8; y = 57; w = 4 ; h = 10;
            //rect_fill_add(p+(X_PITCH*y)+(x&~3), 0x00000010, w>>2, h, X_PITCH-w, 0);
        }
        
        // draw record indicator
        if ((mainprops.lq_mode == false) && (fmod(t, 0.6) < 0.3)) {
            rect_blit_add((uint8_t*)surf.data + X_PITCH*(12) + (X_RES-24), (uint8_t*)common_resources::rec_sprite->data, 8/4, 8, (X_PITCH - 8), 0);
        }
        // draw play mode indicator
        rect_blit_add(
            (uint8_t*)surf.data + X_PITCH*(11) + 12,
            playmode_sprite + player_ctx.playmode*8*8,
            8/4, 8, (X_PITCH - 8), 0
        );

        // rect rect rect!
        //rect_fill_avg((uint8_t*)surf.data + (X_PITCH*52) + 4, 0x20202020, (120)/4, 120, X_PITCH-((120/4)*4), 0);

        // draw circle
        if (!mainprops.lq_mode) linedraw_calc_aa_table(32);
        {
            uint32_t style = mainprops.lq_mode ? LINE_STYLE_ADD : LINE_STYLE_ADD_AA;
            const int TOTAL_SEGS = 72;
            float a = (t*0.1), da = 2.0*pi/(float)TOTAL_SEGS;
            float dr = 1.0+0.1*sin(t*1.4);
            for (int i = 0; i < TOTAL_SEGS; i++) {
                float r1 = 32+vu_bars[i][0];
                float r2 = 36+((vu_bars[i][0]+vu_bars[i][1])*0.5);
                float dar = 0;
                vec2x p[2];
                p[0].x = (((r1+dar) * cos(a)) + (240)) * 65536.0;
                p[0].y = (((r1+dar) * sin(a)) + (105)) * 65536.0;

                p[1].x = (((r2*dr+dar) * cos(a)) + (240)) * 65536.0;
                p[1].y = (((r2*dr+dar) * sin(a)) + (105)) * 65536.0;

                if (lineclip((vec2i*)&p[0], (vec2i*)&p[1], (X_RES-1)<<16, (Y_RES-1)<<16) == 0)
                    linedraw_subpixel((uint8_t*)surf.data, surf.pitch, &p[0], &p[1], 32, style);

                dar += 4+1*sin(t*2.4+a*0.2);
                a += da;
            }
        }

        // draw length indicator
        {
            int length = ((129 * (player_ctx.opm.pos.frame << 11)) / (player_ctx.frames_total >> 5));
            vec2x li[2] = {
                {(20<<16), (155<<16)},
                {(20<<16) + length, (155<<16)},
            };
            linedraw_subpixel((uint8_t*)surf.data, surf.pitch, li, li+1, 48, LINE_STYLE_ADD);

            // draw counter
            char tempstr[40];
            int seconds       = player_ctx.opm.pos.frame / (0x1234dd / player_ctx.opm.header.frame_rate);
            int seconds_total = player_ctx.frames_total  / (0x1234dd / player_ctx.opm.header.frame_rate);
            snprintf(tempstr, sizeof(tempstr), "%01d:%02d/%01d:%02d", 
                seconds / 60, seconds % 60,
                seconds_total / 60, seconds_total % 60
            );
            font_draw_string((uint8_t*)surf.data, &common_resources::smallfont, tempstr, 104, 152, surf.pitch);
        }
        

        // draw mesh (put to draw list actually)
        vec4f l = norm(-origin);
        mesh_draw(mesh_flist, mesh_zmap, mesh, rot, clipPlanes, 0, 120, l, 3.0);

        // draw module selector
        draw_module_selector((uint8_t*)surf.data, t);

        // draw logo!
        rect_blit_mask(
            (uint8_t*)surf.data + (X_PITCH*3) + ((X_RES-256)>>1),
            logo_bitmap_masked,
            256/4,
            41,
            X_PITCH-256,
            0
        );

        // 
        if (!mainprops.lq_mode) linedraw_calc_aa_table(24);
        uint32_t wire_style = mainprops.lq_mode ? LINE_STYLE_ADDS_128 : LINE_STYLE_ADDS_128_AA;
#if 1
        // draw linked list
        facelist_t *flit = &mesh_flist[zsort(mesh_flist, mesh_zmap)];
        int k = 0;
        do {
            switch (flit->style & FACE_TYPE_MASK) {
                case FACE_POLY:
                    facedraw_flat(flit);
                    if (flit->style & POLY_STYLE_WIREFRAME) {
                        linedraw_subpixel((uint8_t*)surf.data, surf.pitch, (vec2x*)&flit->v[0].p, (vec2x*)&flit->v[1].p, 15, wire_style);
                        linedraw_subpixel((uint8_t*)surf.data, surf.pitch, (vec2x*)&flit->v[1].p, (vec2x*)&flit->v[2].p, 15, wire_style);
                        linedraw_subpixel((uint8_t*)surf.data, surf.pitch, (vec2x*)&flit->v[2].p, (vec2x*)&flit->v[0].p, 15, wire_style);
                    }
                    break;
                default:        break;
            }

            flit = flit->next;
        } while (flit != NULL);
#endif

        scroller_draw(&scroller_desc, (uint8_t*)surf.data, 0, Y_RES-7, X_RES, surf.pitch);

        // advance it
        int advance = 2;
        if (fabs(ot - mainprops.frame_time) < 0.4*mainprops.frame_time)
            advance = 2;
        else
            advance = max(((2*ot)/mainprops.frame_time), 1);
        scroller_advance(&scroller_desc, advance);

#ifndef WIN32
        if (show_cpu_time) {
            outp(0x3c9, 0);outp(0x3c9, 0);outp(0x3c9, 0);outp(0x3c9, 0);
        }
#endif
        if (show_debug_info) print_debug((uint8_t*)surf.data, surf.pitch, ptc_getfloat() - frt);

        // render on screen
        ptc_update(surf, PTC_BLT_WAIT | PTC_BLT_PALETTE);
        ptc_flip(PTC_FLIP_WAIT);
        frt = ptc_getfloat();
        frame_counter++;

        // wait for 60fps lock
#ifdef WIN32
        while (t + 1.0/60.0 > ptc_getfloat()) {};
#endif
    }
}

};

