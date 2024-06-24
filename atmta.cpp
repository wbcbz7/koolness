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

#include "main.h"
#include "atmta.h"
#include "introrc.h"
#include "gridlerp.h"

// rocket stuff
#include "include/rocket/rocket.h"

//#define WIN32

#define BPM   180
#define RPB   8
#define TRACKS_DIR "rocket\\"
const double ROWS_PER_SECOND = ((BPM / 60.0f) * RPB);

#ifdef WIN32
#define REMOTE
#include "include/win32/bass.h"

static bool bass_paused = false;
static double current_time = 0.0;
static HSTREAM bass_chan;

bool bass_start() {
    // check the correct BASS was loaded
	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		printf("incorrent BASS version!\n");
		return false;
	}

	// try 48000
	if (!BASS_Init(-1, 48000, 0, 0, NULL))
		// try 44100 if failed
		if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
			printf("unable to init BASS.DLL!\n");
			return false;
		}

    // load wav
    bass_chan = BASS_StreamCreateFile(false, "!!!win32_only\\essneuro+.wav", 0, 0, BASS_STREAM_PRESCAN);
	if (bass_chan == 0) {
		printf("unable to load music!\n");
		return false;
	}

    return true;
}

void bass_stop() {
    BASS_ChannelStop(bass_chan);
	BASS_Free();
}

void bass_pause() {
    BASS_ChannelPause(bass_chan);
}

void bass_play() {
    BASS_ChannelPlay(bass_chan, FALSE);
}

double bass_gettime() {
    return bass_paused ? current_time : BASS_ChannelBytes2Seconds(bass_chan, BASS_ChannelGetPosition(bass_chan, BASS_POS_BYTE));
}

void bass_settime(double t) {
    BASS_ChannelSetPosition(bass_chan, BASS_ChannelSeconds2Bytes(bass_chan, t), BASS_POS_BYTE | BASS_POS_SCAN);
    current_time = t;
}

// rocket callbacks
void rocketPause(void* data, int flag) {
    bass_paused = !!flag;
    if (bass_paused) {
        bass_pause();        // pause()!!!!
    }
    else {
        bass_play();
    }
}
void rocketSetRow(void* data, int row) {
    bass_settime(row / ROWS_PER_SECOND);
    current_time = row / ROWS_PER_SECOND;
};

int rocketIsPlaying(void* data) {
    return bass_paused != true;
}

#else

// rocket callbacks
void rocketPause(void* data, int flag) {
}
void rocketSetRow(void* data, int row) {
}
int rocketIsPlaying(void* data) {
    return true;
}

#endif

namespace atmta {

#define X_PITCH 320
const float fov = 120.0;

struct grid_t {
    int32_t u, v;       // no lighting
};

enum {
    GRID_SIZE = 8,
    X_GRID = (X_RES / GRID_SIZE),
    Y_GRID = (Y_RES / GRID_SIZE),

    X_RES_GRID = X_RES,
    Y_RES_GRID = Y_RES,
};

// private part resources
ptc_palette *pal;
ptc_surface *back0, *back1, *back;

// blur transpose buffer
ptc_surface *transbuf;

// lowres buffer
ptc_surface *lowres;

// texture
uint8_t *texture;

// grid
grid_t *gr;

// temp buffers
unsigned long *tmp, *tmp2;

// rocket instance
Rocket *rocket;

int init() {
    // init rocket
    // prepare callbacks for rocket
    sync_cb cb;
    cb.pause = rocketPause;
    cb.set_row = rocketSetRow;
    cb.is_playing = rocketIsPlaying;
    
    // init rocket
    rocket = new Rocket(BPM, RPB, cb);

#ifdef REMOTE
    rocket->open(TRACKS_DIR, "127.0.0.1");
#else
    rocket->open(TRACKS_DIR);
#endif

    // create second back surface
    // create transpose buffer as attached surface
    back0 = new ptc_surface;
    if (!back0) return 1;
    back0->create(X_RES, Y_RES, 512, PTC_SURFACE_INDEX8, 1, 0);

    pal = new ptc_palette(256, 32);
    if (!pal) return 1;

#if 1
    // generate palette
    argb32 pal0, pal1;
    pal0.v = 0x000810;
    pal1.v = 0xE0F4FF;
    vec3f cpow = {1,0.8,0.8};
    //pal_lerp((argb32*)pal.data, &pal0, &pal1, 1, 127);
    pal_calc(pal, pal0, pal1, cpow, 0, 128, false);
    for (int i = 128; i < 255; i++) pal->data[i] = pal->data[127];
#endif

    // make grid
    gr = new grid_t[((X_RES/GRID_SIZE) + 1) * ((Y_RES/GRID_SIZE) + 1)];

    // load texture
    BITMAPV5HEADER bmphead;
    if (bmp_load8_header("assets\\kaleido.bmp", &bmphead) != 0) {
        printf("can't load texture!\n");
        return 1;
    }
    if (abs(bmphead.bV5Height) != 256 && bmphead.bV5Width != 256) {
        printf("dimensions must be 256x256!\n");
        return 1;
    }
    texture = new uint8_t[bmphead.bV5Height * bmphead.bV5Width];
    if (bmp_load8_data("assets\\kaleido.bmp", &bmphead, texture, NULL, -1)) {
        printf("can't load texture!\n");
        return 1;
    }

    // preload ALL rocket tracks
    volatile float preload;
    preload = rocket->getTrack("kaleido#rotConstVel", 0);
    preload = rocket->getTrack("kaleido#rotSinSwing", 0);
    preload = rocket->getTrack("kaleido#rotSinVel", 0);
    preload = rocket->getTrack("kaleido#angle", 0);
    preload = rocket->getTrack("kaleido#pow", 0);
    preload = rocket->getTrack("kaleido#scale", 0);
    preload = rocket->getTrack("kaleido#subdiv", 0);
    preload = rocket->getTrack("kaleido#dispSpeedX", 0);
    preload = rocket->getTrack("kaleido#dispSwingX", 0);
    preload = rocket->getTrack("kaleido#dispSpeedY", 0);
    preload = rocket->getTrack("kaleido#dispSwingY", 0);
    preload = rocket->getTrack("kaleido#dispConstX", 0);
    preload = rocket->getTrack("kaleido#dispConstY", 0);
    preload = rocket->getTrack("text#startline", 0);
    preload = rocket->getTrack("text#line", 0);
    preload = rocket->getTrack("text#char", 0);
    preload = rocket->getTrack("kaleido#fadeout", 0);
    preload = rocket->getTrack("kaleido#fadeout_step", 0);
    preload = rocket->getTrack("overlay#max", 0);
    preload = rocket->getTrack("overlay#freq", 0);
    preload = rocket->getTrack("overlay#count", 0);
    preload = rocket->getTrack("glitch#dist", 0);
    preload = rocket->getTrack("glitch#seg", 0);

    return 0;
}

void done() {
    delete[] tmp;
    delete[] tmp2;
    delete back0;
    delete back1;

    delete[] gr;
    delete transbuf;
    delete lowres;
}

glist_t glist[Y_RES];

void drawgrid(grid_t *p, float tt) {
    mat2  rot; 
    rot2(rot, (tt * rocket->getTrack("kaleido#rotConstVel", tt)) +
              (rocket->getTrack("kaleido#rotSinSwing", tt) * sin(tt * rocket->getTrack("kaleido#rotSinVel", tt))*cos(tt * 0.3*rocket->getTrack("kaleido#rotSinVel", tt))));
    vec2f direction;
    
    double angle  = rocket->getTrack("kaleido#angle", tt) * pi;
    double powy   = rocket->getTrack("kaleido#pow", tt) * 0.5;  // compensate for sqrt
    double scale  = rocket->getTrack("kaleido#scale", tt);
    double subdiv = rocket->getTrack("kaleido#subdiv", tt);
    
    // generate displacement
    vec2f disp;
    vec2f dispSpeed = {rocket->getTrack("kaleido#dispSpeedX", tt), rocket->getTrack("kaleido#dispSpeedY", tt)}; 
    vec2f dispSwing = {rocket->getTrack("kaleido#dispSwingX", tt), rocket->getTrack("kaleido#dispSwingY", tt)};
    
    vec2f dispConst = {rocket->getTrack("kaleido#dispConstX", tt), rocket->getTrack("kaleido#dispConstY", tt)};

    disp.x = (dispConst.x + dispSwing.x * cos(tt * dispSpeed.x)) * 256.0;
    disp.y = (dispConst.x + dispSwing.y * sin(tt * dispSpeed.y)) * 256.0;
    
    float f = pi / (subdiv + 10e-6);
    float scale16f = 65536.0 * scale;

    direction.y = -(Y_RES_GRID/2)*(0.5 / X_RES_GRID);
    for (int y = 0; y < (Y_RES_GRID/GRID_SIZE)+1; y++) {
        direction.x = -(X_RES_GRID/2)*(0.5 / X_RES_GRID);
        for (int x = 0; x < (X_RES_GRID/GRID_SIZE)+1; x++) {
            float fu, fv;
            
            // set direction vector
            //direction.x = (float)((x * GRID_SIZE) - (X_RES_GRID/2))*(0.5 / X_RES_GRID);
            //direction.y = (float)((y * GRID_SIZE) - (Y_RES_GRID/2))*(0.5 / X_RES_GRID);
            
            float a = atan2(direction.x, direction.y) + angle;
            float r = pow(abs2(direction), powy) * scale16f;
            
            a = fabs(fmod(fabs(a) + f/2.0, f) - f/2.0);
            
            // float u/v coords
            vec2f uv;
            uv.x = cos(a) * r;
            uv.y = sin(a) * r;
            
            mul(uv, rot, uv);
            uv.x += disp.x; uv.y += disp.y;
            
            signed long u = fistf(uv.x);
            signed long v = fistf(uv.y);
            
            p->u = (unsigned long)u & 0xFFFFFFFF;
            p->v = (unsigned long)v & 0xFFFFFFFF;
            
            p++;
            direction.x += GRID_SIZE*(0.5 / X_RES_GRID);
        }
        direction.y += GRID_SIZE*(0.5 / X_RES_GRID);
    }
}

void free() {

}

const char *bottom_text[] = {
    ".koolnESS - an ESFM musicdisk",
    ".by the furnace posse",
    ".code :: design by wbcbz7",
    ".featuring music by abstract64.natt.raijin.djtubig",
    ".spinningsquarewaves.potajoe.gtr3qq.tapekeeper.laggy",
    ".released at multimatograf 2o24.",
};

// write text
void draw_bottom_text(ptc_surface *surf, float t) {
    int startline = rocket->getTrack("text#startline", t);
    int textline = rocket->getTrack("text#line", t);
    int textchar = rocket->getTrack("text#char", t);

    // draw full lines
    int yy = Y_RES-8-8*(textline-startline);
    int xx = 8;
    for (int l = startline; l < textline; l++) {
        font_draw_string_add((uint8_t*)surf->data, &common_resources::smallfont, bottom_text[l], xx, yy, surf->pitch, 0x20202020);
        yy += 8;
    }
    // draw partial line
    for (int i = 0; i < min(strlen(bottom_text[textline]), textchar); i++) {
        xx += font_draw_char_add((uint8_t*)surf->data, &common_resources::smallfont, bottom_text[textline][i], xx, yy, surf->pitch, 0x20202020);
    }
    if (fmod(t, 0.5) < 0.2) {
        font_draw_char_add((uint8_t*)surf->data, &common_resources::smallfont, '_', xx, yy, surf->pitch, 0x20202020);
    }
#if 0
    if ((startline != 0) || (textline != 0))
        rect_fill_avg((uint8_t*)surf->data + (surf->pitch*(Y_RES-8-8)) + 8, 0x00000000, (X_RES-16)/4, 16, surf->pitch-(X_RES-16), 0);
#endif
}

void run() {
#ifdef WIN32
    bass_start();
    bass_play();
#endif

    int frame_counter = 0;
    double t = ptc_getfloat(), st = t, frt = t;

    int to_fadeout = 0;
    float fadeout_step = 0;
    while ((!kbhit())) {
        double nt = ptc_getfloat(), ot = nt - t; t = nt;
        double mt;

#ifdef WIN32
        mt = bass_gettime();
#else
        mt = player_ctx.opm.pos.frame / 60.0; // oof
        if (player_ctx.opm.pos.frame >= 1920) break;
#endif
        t = mt;

        // fetch rocket data
        if (rocket->isRemote()) {
            rocket->update(mt);
        }

        to_fadeout = rocket->getTrack("kaleido#fadeout", t);
        fadeout_step = clamp(rocket->getTrack("kaleido#fadeout_step", t), 0, 1) * 255;
        int overlay_max   = rocket->getTrack("overlay#max", t);
        int overlay_freq  = rocket->getTrack("overlay#freq", t);
        int overlay_count = rocket->getTrack("overlay#count", t);
        int glitch_dist   = rocket->getTrack("glitch#dist", t);
        int glitch_seg    = rocket->getTrack("glitch#seg", t);

        // clear buffer
        ptc_memset(back0->data, 0, back0->pitch * back0->height, 0);

        // copy palette
        memcpy(back0->pal->data, pal->data, sizeof(argb32)*256);
        // generate palette fade
        switch(to_fadeout) {
            case 1: {
                argb32 fadecol; fadecol.v = 0xFFFFFF;
                pal_fade(back0->pal, back0->pal, fadecol, fadeout_step, 0, 256);
                break;
            }
            case 2: {
                argb32 fadecol; fadecol.v = 0x000000;
                pal_fade(back0->pal, back0->pal, fadecol, fadeout_step, 0, 256);
                break;
            }
            default:
                break;
        }

        // render overlay
        overlay_trendwhore(t, common_resources::overgrid, (X_RES / OVR_GRID_SIZE), (Y_RES / OVR_GRID_SIZE),
            1, overlay_max, overlay_freq, overlay_count);

        // draw grid
        drawgrid(gr, t);

        // render grid
        grid_tmap_8(back0->data, gr, texture, (Y_RES_GRID/GRID_SIZE));

        // do overlay
        do_overlay(back0, glist, glitch_dist, glitch_seg, t);

        // draw bottom text
        draw_bottom_text(back0, t);

        // render on screen
        ptc_update(*back0, PTC_BLT_WAIT | PTC_BLT_PALETTE);
        ptc_flip(PTC_FLIP_WAIT);
        frt = ptc_getfloat();
        //printf("%f, %f\n", ot, mainprops.frame_time);
        frame_counter++;

        // wait for 60fps lock
#ifdef WIN32
        while (t + 1.0/60.0 > ptc_getfloat()) {};
#endif
    } 
    if (kbhit()) getch();
#ifdef WIN32
    bass_stop();
#endif
}


}
