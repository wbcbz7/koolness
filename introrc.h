#include <stdint.h>
#include "flexptc.h"
#include "fontdraw.h"
#include "player.h"

// actually sort of xorshift* :)
inline unsigned long xorshift32(unsigned long seed) {
    unsigned long x = seed;
    x ^= x >> 6;  // a
	x ^= x << 12; // b
	x ^= x >> 15; // c
    return (x * 0xB5ED7441);
}

namespace intro_resources {

// common resources here
extern ptc_surface *alpha_overlay;
extern ptc_surface *dot_overlay;


int init();
void done();

}

namespace common_resources {

// common resources here
extern ptc_surface *rec_sprite;
extern uint8_t     *overgrid;      // "mophion gruphix" grid

// fonts
extern font_info_t bigfont;
extern font_info_t smallfont;

#define OVR_GRID_SIZE 16

int init();
void done();

}

int spritegrid_draw(ptc_surface *dst, ptc_surface *src, uint8_t *grid, uint32_t gridsize, uint32_t xgrid, uint32_t ygrid);
int overlay_trendwhore(float t, uint8_t *grid, uint32_t xgrid, uint32_t ygrid,
    int overlayType, int overlayMax, int overlayFreq, int overlayCount);

int do_overlay(ptc_surface *dst, float t);

// recycling old 2016 code - because my newer one is actually harder to integrate! :D
struct glist_t {
    int active, disp;
    int mask1, mask2;
};
int do_overlay(ptc_surface *dst, glist_t *glist, int glvel, int glsize, float t);
void drawflare(ptc_surface &dst, uint8_t *src, vec2i &f, int ures, int vres, uint32_t colormask);
