#pragma once
#include <stdint.h>

// global defines
#define X_RES 320
#define Y_RES 200

struct main_props_t {
    // global switches here

    // fast esfm write mode
    // this MUST BE FIRST member of the structure!
    bool fast_esfm_out;

    // automatic mode select
    bool automode;

    // low quality mode - remove some flashy stuff
    bool lq_mode;

    // frame time in seconds - used for deriving refresh rate
    float frame_time;

    // use IRQ0 timing instead of RTC
    // NB: IRQ0 is still hooked to ensure RTC IRQ is always unmasked!
    bool use_irq0;

    // load modules from disk instead of from memory
    bool load_from_disk;

    // skip intro for 4 MB RAM machines (and 386 :D)
    bool skip_intro;
};

extern "C" {
    extern main_props_t mainprops;
}

// dos shell (yes!)
void dos_shell();
