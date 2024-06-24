#ifndef __PTC_FLEXPTC_H
#define __PTC_FLEXPTC_H

/**
    flexptc type 3 - user include
    by wbcbz7 zb.o3.0x7e1
    
**/

typedef unsigned long PTC_RESULT;

// win32 only
#ifdef __WINDOWS_386__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>

#endif

#define __PTC_CUSTOM_MEMALLOC
#ifdef __PTC_CUSTOM_MEMALLOC
#include "memalloc.h"
#endif


// main stuff

#define PTC_USEVBE          (0 << 0)    // use VESA BIOS Extensions if avail
#define PTC_NOVBE           (1 << 0)    // ignore 'em

#define PTC_USEMMX          (0 << 1)    // use MMX if possible
#define PTC_NOMMX           (1 << 1)    // force to not use MMX

#define PTC_USETSC          (0 << 2)    // use TSC for timing if possible
#define PTC_NOTSC           (1 << 2)    // force to not use TSC

#define PTC_USELINEAR       (0 << 3)
#define PTC_NOLINEAR        (1 << 3)

#define PTC_USEBANKED       (0 << 4)
#define PTC_NOBANKED        (1 << 4)

#define PTC_USECONVERT      (0 << 5)
#define PTC_NOCONVERT       (1 << 5)

#define PTC_ALLOWAUTO       (0 << 6)
#define PTC_FORCEMANUAL     (1 << 6)    // force manual setup

#define PTC_ALLOWCUSTOM     (0 << 7) 
#define PTC_NOCUSTOM        (1 << 7)    // don't try to create vbe 3.0+ custom modes

#define PTC_8BITPAL         (0 << 8) 
#define PTC_6BITPAL         (1 << 8)    // force using standard 18bpp - 6 bit palette

#define PTC_USEVGA          (0 << 9) 
#define PTC_NOVGA           (1 << 9)    // do not enumerate VGA/Mode-X modes

#define PTC_FINDBEST        (0 << 10) 
#define PTC_NOBEST          (1 << 10)   // do not find best matching mode

#define PTC_FORCECUSTOM     (1 << 11)   // create custom mode even if best mode is found

#define PTC_VGACOMPAT       (1 << 12)   // force VGA compatibility for non-VGA controllers/modes (UNSTABLE!)

#define PTC_NOFPUMEMCPY     (1 << 13)   // disable FPU memcpy

#define PTC_ALLOWMULTISCAN  (1 << 14)   // allow multiscan modes (i.e. 320x180 -> 320x720 for 720p compatible timings)

#define PTC_TIMERHANDLER    (1 << 16)   // enable ptc_settimer() for periodic procedure calls (enables RTC IRQ on Pentium and higher)

#define PTC_ALLOWWINDOWED   (0 << 31) 
#define PTC_NOWINDOWED      (1 << 31)   // disable windowed modes

// global return codes

enum {
    ptc_errOK = 0,            // all is OK
    ptc_err_NOTIMPLEMENTED,   // feature is not implemented
    ptc_err_NULLPTR,          // null pointer is passed
    ptc_err_MEMALLOC,         // memory allocation error
    ptc_err_UNKNOWNDEVICE,    // unknown device
    ptc_err_VBEERROR = 0x300, // VBE error (low byte - error code)
    
    ptc_err_USERBREAK = -21,        // user break
    ptc_err_NOFRAMEBUFFER = -20,    // can't get to framebuffer
    ptc_err_NOCONVERT = -19,        // pixel format converter is absent
    ptc_err_NOVIDEOMEM = -18,       // insufficient video memory
    ptc_err_CANTFIND = -12,         // can't find suitable mode (internal)
    ptc_err_ADDMODE = -11,          // ptc_addmode() error (internal)
    ptc_err_WIN32ERR = -10,         // Win32 interface error
    ptc_err_DDRAWERR = -9,          // ddraw error
    ptc_err_UNKNOWN = -8,            // unknown error
};
    
PTC_RESULT ptc_init(unsigned long);
PTC_RESULT ptc_done();

// general purpose memcpy\memset

// convert.asm imports
#pragma aux __convcall "*_\\conv" parm caller [edi] [esi] [ecx] [edx] \
                        value [eax] modify [eax ecx edx esi edi]

#define __convcall __declspec(__pragma("__convcall"))

typedef int __convcall (*PTC_CONVERTER) (void*, void*, unsigned long, unsigned long);

extern "C" {

// memcpys
int __convcall ptc_fpu_memcpy                  (void*, void*, unsigned long, unsigned long);
int __convcall ptc_mmx_memcpy                  (void*, void*, unsigned long, unsigned long);
int __convcall ptc_memcpy                      (void*, void*, unsigned long, unsigned long);

// memsets
int __convcall ptc_mmx_memset                  (void*, unsigned long, unsigned long, unsigned long);
int __convcall ptc_memset                      (void*, unsigned long, unsigned long, unsigned long);

}

// surface.h

union ptc_argb32 {
    struct {
        unsigned char b, g, r, a;
    };
    unsigned long val;
};

// for complex surfaces
struct ptc_surface_request {
    unsigned long   x_res;
    unsigned long   y_res;
    unsigned long   pitch;
    unsigned long   fmt;
    unsigned long   flags;
};

struct ptc_palette {
    ptc_argb32     *data;   // palette data        
    unsigned long   bpp;    // bits per pixel, padded to NEAREST byte -> 8 bit boundary!
    unsigned long   len;    // table length
    unsigned long   rsvd;   // reserved
        
    unsigned long   r_mask; // red   mask
    unsigned long   g_mask; // green mask
    unsigned long   b_mask; // blue  mask
    unsigned long   a_mask; // alpha mask
    
     ptc_palette();
     ptc_palette(unsigned long len, unsigned long bpp);
     ptc_palette(unsigned long len, unsigned long bpp, unsigned long r_mask, unsigned long g_mask, unsigned long b_mask);
    ~ptc_palette();
};

class ptc_surface {
    
    public:
    
    void           *data;       // start pointer
    unsigned long   width;
    unsigned long   height;
    unsigned long   bpp;        // bits per pixel, padded to NEAREST byte -> 8 bit boundary!
    
    unsigned long   flags;      // surface flags
    unsigned long   banks;      // number of banks\bitplanes
    unsigned long   banklen;    // bank\plane length in bytes
    unsigned long   pages;      // number of image pages
    
    unsigned long   pitch;      // bytes per scanline
    unsigned long   pagelen;    // page length in bytes
    
    signed   long   origin_x;
    signed   long   origin_y;
    
    unsigned long   r_mask;     // red   mask
    unsigned long   g_mask;     // green mask
    unsigned long   b_mask;     // blue  mask
    unsigned long   a_mask;     // alpha mask
    
    unsigned long   rect_x;     // rect start x +-------------------+
    unsigned long   rect_y;     // rect start y |  *------+         |
    unsigned long   rect_w;     // rect width   |  |      |h        |
    unsigned long   rect_h;     // rect height  |  +------+         |
                                //              |      w            |
                                //              +-------------------+
                                // * - rect start (rect_x; text_y); w - rect_w; h = rect_h;
    
    unsigned long   curbank;    // current bank
    unsigned long   curpage;    // current page
    
    ptc_palette    *pal;        // palette info
    
#ifdef __PTC_CUSTOM_MEMALLOC
    MemoryAllocator    *alloc;          // allocator pointer
    unsigned long       complex_count;  // complex surface count
    ptc_surface       **complex;        // complex surface pointer table
#endif
    
    ptc_surface();
    ~ptc_surface();
    
    PTC_RESULT create(unsigned int xres, unsigned int yres, unsigned int fmt, unsigned int pages, unsigned int flags);
    
#ifdef __PTC_CUSTOM_MEMALLOC
    PTC_RESULT create(unsigned int xres,  unsigned int yres,  unsigned int pitch, unsigned int fmt,
                      unsigned int pages, unsigned int flags, unsigned long align = 0, MemoryAllocator *alloc = NULL, ptc_surface_request *complex = NULL);
#else
    PTC_RESULT create(unsigned int xres, unsigned int yres, unsigned int pitch, unsigned int fmt, unsigned int pages, unsigned int flags);
#endif
    
    virtual void        * lock();
    virtual void        * lockrect();
    virtual PTC_RESULT    unlock();
    
    PTC_RESULT rect(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    
};

#define PTC_SURFACE_LINEAR          (0 << 0)    // linear as is
#define PTC_SURFACE_BANKED          (1 << 0)    // e.g. vesa 1.x banked
#define PTC_SURFACE_PLANAR          (2 << 0)    // e.g. mode-x
#define PTC_SURFACE_MEMTYPE         (3 << 0)    // memory type mask

#define PTC_SURFACE_INDEXCOLOR      (0 << 2)    // indexed color
#define PTC_SURFACE_DIRECTCOLOR     (1 << 2)    // direct color (hi\true color)

#define PTC_SURFACE_LOCAL           (0 << 3)    // located in system memory
#define PTC_SURFACE_FRAMEBUFFER     (1 << 3)    // located in video  memory

#define PTC_SURFACE_NOALPHA         (0 << 4)    // alpha channel is absent
#define PTC_SURFACE_ALPHA           (1 << 4)    // alpha channel is present

#define PTC_SURFACE_DIRECTALPHA     (0 << 5)    // direct   alpha [min - full transparent, max - full opaque]
#define PTC_SURFACE_ONEMINUSALPHA   (1 << 5)    // reversed alpha [min - full opaque, max - full transparent]

#define PTC_SURFACE_NOPALETTE       (0 << 6)    // palette is absent
#define PTC_SURFACE_PALETTE         (1 << 6)    // palette is present

#define PTC_SURFACE_ALLOCATOR       (1 << 7)    // custom allocator is used

#define PTC_SURFACE_COMPLEX         (1 << 8)    // complex surface
#define PTC_SURFACE_MEMORYSHARING   (1 << 9)    // surface shares memory with primary one

#define PTC_SURFACE_LOCKED          (1 << 31)   // lock in progress

// common surface formats
// many of them are unsupported and will NOT be supported :D
enum {
    PTC_SURFACE_NULL,           // null  format
    PTC_SURFACE_INDEX8,         // 8bpp  indexed
    PTC_SURFACE_GRAY8,          // 8bpp  grayscale
    PTC_SURFACE_R3G3B2,         // 8bpp  RGB332
    
    PTC_SURFACE_X4R4G4B4,       // 16bpp (4bpp reserved + 12bpp RGB444)
    PTC_SURFACE_A4R4G4B4,       // 16bpp (4bpp alpha    + 12bpp RGB444)
    PTC_SURFACE_X1R5G5B5,       // 16bpp (1bpp reserved + 15bpp RGB555)
    PTC_SURFACE_A1R5G5B5,       // 16bpp (1bpp alpha    + 15bpp RGB555)
    PTC_SURFACE_R5G6B5,         // 16bpp RGB565
    
    PTC_SURFACE_R8G8B8,         // 24bpp RGB888
    PTC_SURFACE_B8G8R8,         // 24bpp BGR888
    
    PTC_SURFACE_X8R8G8B8,       // 32bpp (8bpp reserved + 24bpp RGB888)
    PTC_SURFACE_A8R8G8B8,       // 32bpp (8bpp alpha    + 24bpp RGB888)
    
    PTC_SURFACE_X8B8G8R8,       // 32bpp (8bpp reserved + 24bpp BGR888)
    PTC_SURFACE_A8B8G8R8,       // 32bpp (8bpp alpha    + 24bpp BGR888)
    
    // special formats
    PTC_SURFACE_DEPTH8,
    PTC_SURFACE_DEPTH16,
    PTC_SURFACE_DEPTH32,
    
    PTC_SURFACE_STENCIL8,
};

PTC_RESULT  ptc_surface_create(ptc_surface& p, unsigned int xres, unsigned int yres, unsigned int fmt, unsigned int pages, unsigned int flags);
PTC_RESULT  ptc_surface_create(ptc_surface& p, unsigned int xres, unsigned int yres, unsigned int pitch, unsigned int fmt, unsigned int pages, unsigned int flags);
void *      ptc_surface_lock(ptc_surface& p);
void *      ptc_surface_lockrect(ptc_surface& p);
void        ptc_surface_unlock(ptc_surface& p);
PTC_RESULT  ptc_surface_rect(ptc_surface& p, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
void *      ptc_surface_setbank(ptc_surface& p, unsigned int page, unsigned int bank);
PTC_RESULT  ptc_surface_delete(ptc_surface& p);

#define PTC_BLT_NOWAIT              (0 << 0)    // no wait for retrace
#define PTC_BLT_WAIT                (1 << 0)    // no wait for retrace
#define PTC_BLT_SMARTWAIT           (1 << 2)    // use smart wait

#define PTC_BLT_ALPHA               (0 << 3)    // use alpha channel if available
#define PTC_BLT_IGNOREALPHA         (1 << 3)    // ignore alpha channel

#define PTC_BLT_PALETTE             (1 << 4)    // use palette

// return codes
enum {
    ptc_err_surface_ALIGN = 0x4001, // alignment required
    ptc_err_surface_INCORRECTFMT,   // incorrect surface format
    ptc_err_surface_LOCKED,         // surface is locked
};

// framebuffer-attached surface (can be used for direct-to-backbuffer writes, bypassing ptc_update())
// NB! in case of one-page mode (frontbuffer only) it actually works as frontbuffer and all writes are 
// appearing right at the screen, so check mode flags (single/double buffer) before use and fallback
// to ptc_update() to avoid artifacts!
class ptc_surface_backbuffer : public ptc_surface {
    public:
        ptc_surface_backbuffer() {};
        ~ptc_surface_backbuffer() {};
    
        // overloaded lock()/lockrect()
        // check return pointers and ALWAYS fallback to ptc_update() in case of NULL pointer!
        virtual void * lock();                      // modetweaks-unaware lock - fails if doubled/etc...
        virtual void * lock(unsigned int flags);    // modetweaks-aware lock
        
        // overloaded unlock() - ALWAYS call this when you're done with it
        virtual PTC_RESULT unlock();
};

#define PTC_LOCK_TWEAKAWARE     (1 << 0)

// mode.h

#define PTC_MODE_VGA            PTC_DEVICE_VGA
#define PTC_MODE_VESA           PTC_DEVICE_VESA
#define PTC_MODE_DEVICE         PTC_DEVICE_MASK  // device mask

#define PTC_MODE_USECRTC        (1 << 2)  // use custom CRTC timing blah-blah

#define PTC_MODE_LINEAR         (0 << 3)
#define PTC_MODE_BANKED         (1 << 3)
#define PTC_MODE_PLANAR         (2 << 3)
#define PTC_MODE_MEMTYPE        (3 << 3)  // memory type mask

#define PTC_MODE_VGACOMPAT      (0 << 5)  // vga compatible mode
#define PTC_MODE_NONVGA         (1 << 5)  // vga incompatible

#define PTC_MODE_ALLOWWAIT      (0 << 6)  // use    wait flag for ptc_update()/ptc_flip()
#define PTC_MODE_NOWAIT         (1 << 6)  // ignore wait flag for ptc_update()/ptc_flip()

#define PTC_MODE_AUTOSETUP      (0 << 7)  // use auto setup
#define PTC_MODE_MANUALSETUP    (1 << 7)  // use manual setup

#define PTC_MODE_ASPECT_4_3     (0 << 8)  // use standard 4:3 aspect ratio (default ;)
#define PTC_MODE_ASPECT_16_9    (1 << 8)  // use standard 16:9 aspect ratio
#define PTC_MODE_ASPECT_16_10   (2 << 8)  // use standard 16:10 aspect ratio
#define PTC_MODE_ASPECT_IGNORE  (3 << 8)  // ignore aspect ratio
#define PTC_MODE_ASPECT         (3 << 8)  // aspect ratio mask

#define PTC_MODE_SINGLEBUFFER   (0 << 10) // single buffer (one video page)
#define PTC_MODE_MULTIPLEBUFFER (1 << 10) // multiple buffer (several video pages)

#define PTC_MODE_FORCEBANKED    (1 << 11)

#define PTC_MODE_NOMMX          (0 << 12)
#define PTC_MODE_USEMMX         (1 << 12)

#define PTC_MODE_NONCUSTOM      (0 << 13)
#define PTC_MODE_CUSTOM         (1 << 13) // custom vbe 3.0-based mode (tricky stuff!)

#define PTC_MODE_MULTISCAN      (1 << 14) // multiscan mode (i.e. 320x180 -> 320x720 for 720p compatible timings)

#define PTC_MODE_USE_KEYBOARD   (1 << 15) // provide keyboard input
#define PTC_MODE_USE_MOUSE      (1 << 16) // provide mouse input

#define PTC_MODE_6BITPAL        (0 << 29) 
#define PTC_MODE_8BITPAL        (1 << 29) // use 24bpp (8 bit per channel) extended palette

#define PTC_MODE_SINGLE         (0 << 30) 
#define PTC_MODE_DOUBLED        (1 << 30) // horizontal doubled

#define PTC_MODE_WINDOWED       (1 << 24)

#define PTC_MODE_WIN8HACK       (1 << 25) // use win8/8.1 ddraw fullscreen hack to circumvent 30fps lock issue

#define PTC_MODE_ALLOW_ESC      (1 << 26) // do not close application by pressing escape

#define PTC_MODE_INCORRECT      (1 << 31) // mode params are incorrect or undefined or impossible

typedef signed long PTC_MODEHANDLE;

// general ptc_mode structure (without backend-specific info)
struct ptc_mode {
    char         *title;        // window title
    
    void         *data;         // framebuffer pointer (INTERNAL ONLY, must use surface.lock() to obtain actual pointer!)
                                
    // surface.data = ptc_mode.data + (surface.pitch * ptc_mode.y_padding) +
    //                ((surface.bpp >> 3) * ptc_mode.x_padding);
    
    // virtual mode properties
    unsigned long width;
    unsigned long height;
    unsigned long bpp;          // full-scale bpp (e.g. 15 is valid also ;) 
    unsigned long pages;        // number of image pages
    
    unsigned long dispage;      // displayed page (backpage in surface.curpage)
    
    unsigned long aspect;       // ((x << 16) / y) :)
    
    unsigned long x_padding;    // number of padding pixels
    unsigned long y_padding;    // number of padding scanlines
    
    unsigned long flags;
    unsigned long rate;         // refresh rate
    
    // converter function
    PTC_CONVERTER convproc;
    unsigned long convflags;
    
    // surface info
    ptc_surface_backbuffer   surface;
};

enum {
    ptc_mode_CANTFIND       = -1,       // can't find mode
    ptc_mode_NOTIMPLEMENTED = -2,       // feature not implemented
    ptc_mode_USEREXIT       = -3,       // user exit
    
    ptc_setmode_OUTOFBOUND  = 0xA000,   // mode list index out of bounds
};

PTC_MODEHANDLE ptc_addmode(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
PTC_MODEHANDLE ptc_addmode(char*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);

PTC_RESULT  ptc_setmode(PTC_MODEHANDLE);
PTC_RESULT  ptc_donemode();
PTC_RESULT  ptc_restoremode();

ptc_mode*   ptc_getmodestruct(PTC_MODEHANDLE);
ptc_mode*   ptc_getcurrentmodestruct();

// framebuf.h
void        ptc_messageloop(bool sleep = false);
bool        ptc_isexit();
PTC_RESULT  ptc_wait(unsigned long line);
PTC_RESULT  ptc_update(ptc_surface& src, unsigned int flags);
PTC_RESULT  ptc_flip(unsigned long flags);
PTC_RESULT  ptc_setpal(ptc_palette &pal, unsigned long start, unsigned long length, unsigned long flags);

// hacky but useful flag :)
extern "C" {
extern volatile bool ptc_force_vblank;
}

// define ptc_update return codes
enum {
    ptc_err_update_NOCONVERT      = 0x8001, // undefined converter
    ptc_err_update_INVALIDMEMTYPE,          // invalid surface memory type
    ptc_err_update_INVALIDSIZE,             // framebufer and source surfaces size do not match
    
    ptc_err_flip_NOPAGES = 0x8101,          // no pages in swap chain
    
    ptc_err_setpal_NONINDEXMODE   = 0x8201, // unacceptable in directcolor modes
    
    // more in future...
};

#define PTC_FLIP_WAIT   PTC_BLT_WAIT
#define PTC_SETPAL_WAIT PTC_BLT_WAIT


// timer.h
PTC_RESULT         ptc_inittimer();
PTC_RESULT         ptc_resettimer();
PTC_RESULT         ptc_freetimer();
unsigned long long ptc_getms();
unsigned long long ptc_get16();
double             ptc_getfloat();
PTC_RESULT         ptc_settimer(void (*func)(void*), uint32_t delta, void* parm);

// user input (rudimentaly!)
enum {
    PTC_MOUSE_LEFT   = (1 << 0),
    PTC_MOUSE_MIDDLE = (1 << 1),
    PTC_MOUSE_RIGHT  = (1 << 2),
};
struct ptc_mouse_coords {
    int          x, y, z;        // x/y are window/bound, wheel is free-running
    unsigned int buttons;
};

PTC_RESULT ptc_getmouse(ptc_mouse_coords &coords);
PTC_RESULT ptc_setmouse(ptc_mouse_coords &coords);
PTC_RESULT ptc_mousehide();
PTC_RESULT ptc_mouseshow();
bool       ptc_getkey(int vk, bool clear);

#endif
