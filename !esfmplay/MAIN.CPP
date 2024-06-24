#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdint.h>
#include <i86.h>
#include <math.h>

#include "esfmout.h"
#include "irq0.h"
#include "opmplay.h"
#include "player.h"
#include "textmode.h"

// global(?) stuff
int esfmbase = 0x388;

#define X_RES 320
#define Y_RES 200

// ------------------------
// draw VU-meters
void draw_vu(uint16_t *textbuf) {
    // draw
    int xx = (80 - 72)/2;
    int* bar = player_ctx.bars;
    int* k = player_ctx.key;
    bar_keyofs_t *bar_keyofs = player_ctx.bars_keyofs;
#if 1
    for (int ch = 0; ch < 18; ch++) {
        for (int op = 0; op < 4; op++) {
            int yy = 32 + (player_ctx.bars_offset >> 2) + (bar_keyofs->value >> 2);

            // get height
            int len = *bar + bar_keyofs->value - 12;
            if (len > 63) len = 63;
            if (len < 0)  len = 0;
            len >>= 1;
            if ((yy - len) < 1) len = yy - 1;

            // draw bars
            uint16_t* p = (uint16_t*)textbuf + (80 * yy) + xx;
            uint16_t attr = 0xBA | (*k == 3 ? 0x0F00 : 0x0700);
            for (int i = 0; i < len; i++) {
                *p = attr; p -= 80;
            }
            bar++;

            // advance
            xx++;
        }
        bar_keyofs++;
        k++;
    }
#else
    xx = 0;
    for (int ch = 0; ch < 18; ch++) {
        for (int op = 0; op < 4; op++) {
            textbuf[xx++] = player_ctx.bars[(ch<<2)+op] | 0x0700;
        }
    }
#endif
}


// keyboard UI
void handle_keyboard() {
    int volume;

    // keyboard processing
    if (kbhit()) {
        int ch = getch(); if (ch == 0) ch |= (getch() << 8);
        switch(ch) {
            case 'O': case 'o':
                volume = esfm_get_volume()-1; if (volume < 0) volume = 0;
                esfm_set_volume(volume);
                break;
            case 'P': case 'p':
                volume = esfm_get_volume()+1; if (volume > 15) volume = 15;
                esfm_set_volume(volume);
                break;
            case 27: player_ctx.run = false; break;
            default: break;
        }
    }
}

// ------------------------
// check ESFM presence, prompt user for more actions
uint32_t try_esfm_detect() {
    uint32_t iobase = esfm_detect();
    if (iobase) {
        printf("ESFM detected at base 0x%03X\n", iobase);
    } else {
        printf("ESFM not detected! Continue? <Y/N>\n");
        char ch = getch();
        if (ch == 'Y' || ch == 'y') iobase = 0x388; // try adlib default
    }
    return iobase;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("usage: esfmplay.exe [tune.opm]\n");
        return 1;
    }

    // init player context
    player_init();

    // try ESFM detect
    if ((esfmbase = try_esfm_detect()) == 0) return 1;

    // install IRQ0 timer
    irq0_initTimer();

    // load first module
    if (player_load_module(&player_ctx, argv[1])) {
        printf("unable to load tune!\n");
        irq0_freeTimer();
        return 1;
    };

    // init ESFM
    esfm_set_baseport(esfmbase);
    esfm_enable();
    esfm_reset();

    // set 80x50 mode
    _asm {
        mov ax, 3
        int 0x10
        mov ax, 0x1112
        xor ebx, ebx
        int 0x10;
    }

    // init textmode stuff
    set_textbuf(player_ctx.textbuf);

    // main loop
    while (player_ctx.run) {
        // update loop!
        // warning: totally async! (perhaps fine since we're not doing anything dangerous)
        player_update_view();

        // clear text buffer
        memset(player_ctx.textbuf, 0, sizeof(player_ctx.textbuf));

        // draw position text
        {
            int seconds       = player_ctx.opm.pos.frame / (0x1234dd / player_ctx.opm.header.frame_rate);
            int seconds_total = player_ctx.frames_total  / (0x1234dd / player_ctx.opm.header.frame_rate);
            tprintf(0, 0, "%02d:%02d/%02d:%02d [%05d/%05d]",
                seconds / 60, seconds % 60,
                seconds_total / 60, seconds_total % 60,
                player_ctx.opm.pos.frame, player_ctx.frames_total
            );
        }
        // draw VU
        draw_vu(player_ctx.textbuf);

        // copy to screen
        while (inp(0x3da) & 8); while (!(inp(0x3da) & 8));
        // copy text buffer
        memcpy((void*)0xB8000, player_ctx.textbuf, sizeof(player_ctx.textbuf));

        // handle keyboard
        handle_keyboard();
    };

    // back to 80x25 mode
    _asm {
        mov ax, 3
        int 0x10
    }

fail_done:
    // free IRQ0 timer
    irq0_freeTimer();

    // set ESFM back to OPL3 mode
    esfm_reset();
    esfm_disable();

    // free opmplay
    opmplay_free(&player_ctx.opm);

    return 0;
}
