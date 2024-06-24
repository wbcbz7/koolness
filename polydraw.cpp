#include <stdint.h>
#include "vec.h"
#include "poly.h"

// zpizzheno from fatmap2.zip by MRI\Doomsday just because
// I didn't have ANY time to write my own tmapper :)

static vec3bx * max_vtx;                  // Max y veci (ending veci)
static vec3bx * start_vtx, * end_vtx;     // First and last veci in array
static vec3bx * right_vtx, * left_vtx;    // Current right and left veci

static long right_height, left_height;
static long right_x, right_dxdy, left_x, left_dxdy;
static long left_u, left_dudy, left_v, left_dvdy;
static long _dudx, _dvdx;

// edge buffer
struct edge_buffer_t {
    uint8_t *p;
    int length;
    int u, v;
    int dudx, dvdx;
    int l, dldx;
};
edge_buffer_t edgebuf[256];

// line renderers
#pragma aux __linedrawcall "*_" parm caller [esi] [eax] [ecx] \
                        value [eax] modify [eax ebx ecx edx esi edi]

#define __edgebufdrawcall __declspec(__pragma("__linedrawcall"))

//                                                          esi         eax      esi
//                                                        edgebuf      color    length 
typedef void __edgebufdrawcall (*edgebufdraw_proc_t) (edge_buffer_t*, int32_t, uint32_t);

extern "C" {
    void __edgebufdrawcall edge_draw_flat_mov_a(edge_buffer_t*, int32_t, uint32_t);
    void __edgebufdrawcall edge_draw_flat_avg_a(edge_buffer_t*, int32_t, uint32_t);
};

void edge_draw_flat_mov(edge_buffer_t *edgebuf, int color, int length) {
    do {
        _asm {
            mov     esi, [edgebuf]
            mov     eax, [color]
            mov     edi, [esi + 0*4]
            mov     ecx, [esi + 1*4]
            rep     stosb
        }
        edgebuf++;
    } while (--length);
}

static void RightSection(void)
{
    // Walk backwards trough the veci array

    vec3bx * v2, * v1 = right_vtx;
    if(right_vtx > start_vtx) v2 = right_vtx-1;     
    else                      v2 = end_vtx;         // Wrap to end of array
    right_vtx = v2;

    // v1 = top veci
    // v2 = bottom veci 

    // Calculate number of scanlines in this section

    right_height = ceilx(v2->p.y) - ceilx(v1->p.y);
    if(right_height <= 0) return;

    // Guard against possible div overflows

    if(right_height > 1) {
        // OK, no worries, we have a section that is at least
        // one pixel high. Calculate slope as usual.

        long height = v2->p.y - v1->p.y;
        right_dxdy  = idiv16(v2->p.x - v1->p.x, height);
    }
    else {
        // Height is less or equal to one pixel.
        // Calculate slope = width * 1/height
        // using 18:14 bit precision to avoid overflows.

        long inv_height = (0x10000 << 14) / (v2->p.y - v1->p.y);  
        right_dxdy = imul14(v2->p.x - v1->p.x, inv_height);
    }

    // Prestep initial values

    long prestep = (ceilx(v1->p.y) << 16) - v1->p.y;
    right_x = v1->p.x + imul16(prestep, right_dxdy);
}

static void LeftSection(void)
{
    // Walk forward trough the veci array

    vec3bx * v2, * v1 = left_vtx;
    if(left_vtx < end_vtx) v2 = left_vtx+1;
    else                   v2 = start_vtx;      // Wrap to start of array
    left_vtx = v2;

    // v1 = top vecx
    // v2 = bottom vecx 

    // Calculate number of scanlines in this section

    left_height = ceilx(v2->p.y) - ceilx(v1->p.y);
    if(left_height <= 0) return;

    // Guard against possible div overflows

    if(left_height > 1) {
        // OK, no worries, we have a section that is at least
        // one pixel high. Calculate slope as usual.

        long height = v2->p.y - v1->p.y;
        left_dxdy = idiv16(v2->p.x - v1->p.x, height);
        //left_dudy = idiv16(v2->u - v1->u, height);
        //left_dvdy = idiv16(v2->v - v1->v, height);
    }
    else {
        // Height is less or equal to one pixel.
        // Calculate slope = width * 1/height
        // using 18:14 bit precision to avoid overflows.

        long inv_height = (0x10000 << 14) / (v2->p.y - v1->p.y);
        left_dxdy = imul14(v2->p.x - v1->p.x, inv_height);
        //left_dudy = imul14(v2->u - v1->u, inv_height);
        //left_dvdy = imul14(v2->v - v1->v, inv_height);
    }

    // Prestep initial values

    long prestep = (ceilx(v1->p.y) << 16) - v1->p.y;
    left_x = v1->p.x + imul16(prestep, left_dxdy);
    //left_u = v1->u + imul16(prestep, left_dudy);
    //left_v = v1->v + imul16(prestep, left_dvdy);
}

namespace __tmap {

static void *dst;
static float TopClip, BotClip, LeftClip, RightClip;
static long dst_pitch;

void DrawFlatPoly(vec3bx * vtx, int vertices, int color, int style)
{
    edge_buffer_t *eb = edgebuf;
    start_vtx = vtx;        // First vertex in array

    // Search trough the vtx array to find min y, max y
    // and the location of these structures.

    vec3bx * min_vtx = vtx;
    max_vtx = vtx;

    long min_y = vtx->p.y;
    long max_y = vtx->p.y;

    vtx++;

    for(int n=1; n<vertices; n++) {
        if(vtx->p.y < min_y) {
            min_y = vtx->p.y;
            min_vtx = vtx;
        }
        else
        if(vtx->p.y > max_y) {
            max_y = vtx->p.y;
            max_vtx = vtx;
        }
        vtx++;
    }

    // OK, now we know where in the array we should start and
    // where to end while scanning the edges of the polygon

    left_vtx  = min_vtx;    // Left side starting vertex
    right_vtx = min_vtx;    // Right side starting vertex
    end_vtx   = vtx-1;      // Last vertex in array

    // Search for the first usable right section

    do {
        if(right_vtx == max_vtx) return;
        RightSection();
    } while(right_height <= 0);

    // Search for the first usable left section

    do {
        if(left_vtx == max_vtx) return;
        LeftSection();
    } while(left_height <= 0);

    unsigned char *destptr = ((unsigned char*)dst + (ceilx(min_y) * dst_pitch));

    for(;;)
    {
        long x1 = ceilx(left_x);
        long width = ceilx(right_x) - x1;
        
        unsigned char *p = destptr + x1;
        
#if 0
        if (width > 0)      // here double-sided polys are rejected
            _asm {
                mov     eax, color
                mov     edi, p
                mov     ecx, width
                rep     stosb           // don't! write your own mapper instead!
            }
#else
        if (width > 0) {
            eb->p = p;
            eb->length = width;
            eb++;
        }
#endif

        destptr += dst_pitch;

        // Scan the right side

        if(--right_height <= 0) {               // End of this section?
            do {
                if(right_vtx == max_vtx) goto draw_edgebuf;
                RightSection();
            } while(right_height <= 0);
        }
        else 
            right_x += right_dxdy;

        // Scan the left side

        if(--left_height <= 0) {                // End of this section?
            do {
                if(left_vtx == max_vtx) goto draw_edgebuf;
                LeftSection();
            } while(left_height <= 0);
        }
        else 
            left_x += left_dxdy;
    }

draw_edgebuf:
    // render edge buffer
    int length = eb - edgebuf;
    if (length > 0) {
        edgebufdraw_proc_t proc = edge_draw_flat_avg_a;
        proc(edgebuf, color * 0x01010101, length);
    }
}

}

// ---------------------------------------------

using namespace __tmap;

void tmap_init(void *buf, int xres, int yres, int pitch) {
    dst = buf;
    TopClip = 0; BotClip = yres; LeftClip = 0; RightClip = xres;
    dst_pitch = pitch;
}

void facedraw_flat(facelist_t *f) {
    DrawFlatPoly(&f->v[0], 3, f->c, f->style);
}

    