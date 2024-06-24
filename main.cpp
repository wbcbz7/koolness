#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <ctype.h>
#include <time.h>
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

#include "esfmout.h"
#include "irq0.h"
#include "opmplay.h"
#include "player.h"

#include "main.h"
#include "atmta.h"
#include "menu.h"
#include "introrc.h"

main_props_t mainprops;

PTC_MODEHANDLE gfx_mode;

void close_all() {
#ifndef WIN32
    // free IRQ0 timer
    irq0_freeTimer();

    // set ESFM back to OPL3 mode
    esfm_reset();
    esfm_disable();

    // free opmplay
    opmplay_free(&player_ctx.opm);
#endif
}

#ifndef WIN32
#include <i86.h>
#include <dos.h>
#include <direct.h>
static void (__interrupt __far *irq2F_oldhandler)();  // internal procedure - old INT70h handler
void _interrupt _far int2F_handler (union INTPACK r) {
    if (r.w.ax == 0x1687) {
        // mark DPMI as not present
        return;
    } else _chain_intr(irq2F_oldhandler);
}

// mad stuff!
void dos_shell() {
    ptc_donemode();
    _asm {
        mov eax, 3
        int 0x10
    }
    char *cwd = getcwd(NULL, NULL);
    uint32_t drive, dummy; _dos_getdrive(&drive);
    //printf("cwd = %s\n", cwd);

    // jump to DOS

    printf("type \"exit\" to return back!\n");
    //system("command.com");
    system("%COMSPEC%");

    // return back
    chdir(cwd);
    free(cwd);
    _dos_setdrive(drive, &dummy);
    // reinit gfx mode
    int rtn;
    if ((rtn = ptc_setmode(gfx_mode)) != ptc_errOK) {
        printf("can't open video mode - error %d\n", rtn); 
        close_all();
    }
}
#else 
void dos_shell() {
}
#endif

// ----------------------
// init gfxmode
bool init_gfx() {
#ifdef WIN32
    mainprops.automode = true;
#endif
    int ptcflags  =
        (mainprops.automode ? 0 : PTC_FORCEMANUAL) | 
        (mainprops.use_irq0 ? 0 : PTC_TIMERHANDLER);
    int modeflags = PTC_MODE_MULTIPLEBUFFER | PTC_MODE_DOUBLED | PTC_MODE_WINDOWED;

    int rtn = 0;
    if ((rtn = ptc_init(ptcflags)) != ptc_errOK) {
        printf("can't init flexptc - error %d\n", rtn);
        return true;
    }
    gfx_mode = ptc_addmode(X_RES, Y_RES, 8, 60, modeflags);
    if (gfx_mode < 0) {
        if (gfx_mode != ptc_mode_USEREXIT) printf("can't find 320x200 8bpp video mode - error %d\n", gfx_mode);
        return true;
    }
    return false;
}
bool set_gfx() {
    int rtn;
    if ((rtn = ptc_setmode(gfx_mode)) != ptc_errOK) {
        if (rtn != ptc_mode_USEREXIT) printf("can't open video mode - error %d\n", rtn);
        return true;
    }

    return false;
}

// done gfxmode
bool done_gfx() {
    ptc_done();
    return false;
}

// ------------------------
// check ESFM presence, prompt user for more actions
uint32_t try_esfm_detect() {
#ifndef WIN32
    uint32_t iobase = esfm_detect();
    if (iobase) {
        printf("ESFM detected at base 0x%03X\n", iobase);
    } else {
        printf("ESFM not detected! Continue? <Y/N>\n");
        char ch = getch();
        if (ch == 'Y' || ch == 'y') iobase = 0x388; // try adlib default
    }
    return iobase;
#else
    return 1; // fake
#endif
}

int main(int argc, char* argv[]) {
    int esfmbase;

    mainprops.automode = false;
    mainprops.lq_mode  = false;
    mainprops.use_irq0 = true;
    mainprops.fast_esfm_out = false;
    mainprops.load_from_disk = false;
    mainprops.skip_intro = false;

    printf("koolnESS - an ESFM music disk - the furnace posse - 2o24\n");
    printf("-----------------------------------\n");

    // TODO: command line parsing
    // parse argv
    for (int p = 1; p < argc; p++) {
        if (strstr(argv[p], "?")) {
            printf("auto    - autosetup\n");
            printf("rtc     - use RTC for timing instead of IRQ0\n");
            printf("potato  - lower display quality\n");
            printf("nointro - skip intro (for 4MB machines)\n");
            //printf("lowmem  - load songs directly from disk instead of preloading at start\n");
            printf("fast    - remove delays from ESFM register out (can sound glitchy!)\n");
            return 0;
        }
#if 0
        if (toupper(argv[p][0]) == 'W') {
            mainprops.aspect = true;
        }
#endif
        if (toupper(argv[p][0]) == 'A') {
            mainprops.automode = true;
        }
        if (toupper(argv[p][0]) == 'R') {
            mainprops.use_irq0 = false;
        }
        if (toupper(argv[p][0]) == 'P') {
            mainprops.lq_mode = true;
        }
        if (toupper(argv[p][0]) == 'N') {
            mainprops.skip_intro = true;
        }
#if 0
        if (toupper(argv[p][0]) == 'L') {
            mainprops.load_from_disk = true;
        }
#endif
        if (toupper(argv[p][0]) == 'F') {
            mainprops.fast_esfm_out = true;
        }
    }

#ifndef WIN32
    // try ESFM detect
    if ((esfmbase = try_esfm_detect()) == 0) return 1;

    // install IRQ0 timer
    irq0_initTimer();
#endif

    // initialize RNG
    srand(time(NULL));

    // init flexptc
    if (init_gfx() != false) goto deinit;

    // init player context
    if (player_init(!mainprops.load_from_disk) != false) goto deinit;
    printf("."); fflush(stdout);

    // init parts
    if (common_resources::init()) goto deinit;
    printf("."); fflush(stdout);
    if (menu::init()) goto deinit;
    printf("."); fflush(stdout);
    if (!mainprops.skip_intro) {
        if (intro_resources::init()) goto deinit;
        printf("."); fflush(stdout);
        if (atmta::init()) goto deinit;
    }
    printf("."); fflush(stdout);

    printf("-----------------------\n");
    if (set_gfx() != false) goto deinit;
    ptc_wait(-1);
    ptc_wait(-1);

    // we are in graphics mode now
    // get refresh rate
    {
        // dummy
        ptc_wait(-1);
        float t = ptc_getfloat();
        ptc_wait(-1);
        t = ptc_getfloat() - t;
    }
    mainprops.frame_time = 0;
    for (int i = 0; i < 4; i++) {
        ptc_wait(-1);
        float t = ptc_getfloat();
        ptc_wait(-1);
        mainprops.frame_time += ptc_getfloat() - t;
    }
    mainprops.frame_time *= (1.0 / 4.0);
    // frame limit for scroller
    if (mainprops.frame_time < (1.0 / 77.0)) mainprops.frame_time = (1.0 / 60.);

#ifndef WIN32
    // init ESFM
    esfm_set_baseport(esfmbase);
    esfm_enable();
    esfm_reset();
#endif

    // wait for 1.5 seconds to wait for capture device
    // TODO
    ptc_resettimer();
    while ((ptc_getfloat() < 1.5) && (!kbhit()));
    ptc_resettimer();

    // load first module
    player_load_module_idx(&player_ctx, 0);
    player_ctx.nextsong.loop_count = modulelist[0].loop_count;      // HACK HACK!!!
    //player_load_module(&player_ctx, modulelist[0].file);
    //player_load_module_packed(&player_ctx, modulelist[0].data, modulelist[0].raw_size);
    

    if (!mainprops.skip_intro) {
        // run atmta
        atmta::run();

        // DEINIT wirebars and atmta to free memory
        //wirebars::done();
        atmta::done();
        intro_resources::done();
    }

    // run menu
    menu::run();

    // deinit menu
    menu::done();

    // shut down flexptc
    done_gfx();

deinit:
    // close all stuff
    close_all();

    return 0;
}


