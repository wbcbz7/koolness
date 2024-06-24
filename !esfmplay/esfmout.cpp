#include <i86.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <dos.h>
#include "esfmout.h"
#include "tinypci.h"

static uint32_t esfm_base;
static uint32_t esfm_volume = 11;

void esfm_delay(int timeout) {
    do { _asm {in al, 0xE1} } while (--timeout);
}

void esfm_set_baseport(uint32_t base) {
    esfm_base = base;
}

void esfm_out(int reg, int data) {
    outp(esfm_base + 2, (reg & 0xFF));
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
    outp(esfm_base + 3, (reg >> 8));
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
    outp(esfm_base + 1, data);
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
}

void esfm_enable() {
    outp(esfm_base + 2, 0x05);
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
    outp(esfm_base + 3, 0x81);
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
}

void esfm_disable() {
    outp(esfm_base + 0, 0);
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
}

void esfm_reset() {
    // TODO: confirm
    for (int i = 0; i < 0x7FF; i++) {
        esfm_out(i, 0);
    }
}

bool esfm_try_detect(uint32_t oplbase) {
    bool rtn = false;
    // enable OPL3 mode
    outp(oplbase + 2, 0x05);
    esfm_delay(128);
    outp(oplbase + 3, 0x01);
    esfm_delay(1024);
    // try to enable ESFM native mode
    outp(oplbase + 2, 0x05);
    esfm_delay(128);
    outp(oplbase + 3, 0x80);
    esfm_delay(1024);
    // try to read compatibility register now
    outp(oplbase + 2, 0x05);
    esfm_delay(128);
    outp(oplbase + 3, 0x05); // select ESFM reg 0x505 (or OPL3 bank 1 reg 0x05)
    esfm_delay(1024);
    int ii = inp(oplbase + 1);
    if (ii == 0x80) {
        // it's a ESFM!
        rtn = true;
        // return to OPL3-compatible mode
        outp(oplbase, 0);
    }
    return rtn;
}

void esfm_set_volume(uint32_t volume) {
    if (esfm_base != 0x388) {
        volume &= 15;
        outp(esfm_base + 0x04, 54); esfm_delay(1);
        outp(esfm_base + 0x05, (volume << 4) | (volume)); esfm_delay(1);
    }
    esfm_volume = volume;
}

uint32_t esfm_get_volume() {
    if (esfm_base != 0x388) {
        outp(esfm_base + 0x04, 54); esfm_delay(1);
        esfm_volume = inp(esfm_base + 0x05) & 0x0F; esfm_delay(1);
    }
    return esfm_volume;
}


static void esfm_activate(uint32_t iobase) {
    // activate ESFM
    // https://github.com/pachuco/ESSPlayMid/blob/master/src/esfmmidi.c#L81
    outp(iobase + 0x04, 72); esfm_delay(1);     // serial mode control
    outp(iobase + 0x05, 0); esfm_delay(1);
    outp(iobase + 0x04, 127); esfm_delay(1);    // music digital record
    outp(iobase + 0x05, 0); esfm_delay(1);
    outp(iobase + 0x04, 107); esfm_delay(1);    // music DAC record volume
    outp(iobase + 0x05, 0); esfm_delay(1);
    outp(iobase + 0x07, 66); esfm_delay(1);     // power management register
}

// searches for ESFM, returns base address
uint32_t esfm_detect() {
    uint32_t iobase = 0; 

    char envstr[128];

    // query BLASTER variable
    char* blasterEnv = getenv("BLASTER");
    if (blasterEnv != NULL) {
        // copy variable to temporary buffer
        strncpy(envstr, blasterEnv, sizeof(envstr));

        // tokenize
        char* p = strtok(envstr, " ");
        while (p != NULL) {
            switch (toupper(*p)) {
                case 'A': iobase = strtol(p + 1, NULL, 16); break;
                default: break;
            }
            p = strtok(NULL, " ");
        }
    } else {
        // scan for ESS Solo-1
        pciDeviceList pcilist;
        if (tinypci::enumerateByDeviceId(&pcilist, 1, 0x125d, 0x1969) != 0) {
            // extract BAR2
            iobase = tinypci::configReadDword(pcilist.address, 0x14) & ~0xF;
        } else {
            // try default 0x220 :) (danger zone!)
            iobase = 0x220;
        }
    }

    // activate ESFM
    esfm_activate(iobase);

    // try detect here
    if (esfm_try_detect(iobase)) {
        // found!
        esfm_base = iobase;
        // unmute if muted
        // TODO: detect dosbox-x somehow (it's too quiet now!)
        if (esfm_get_volume() == 0) esfm_set_volume(11);
        return iobase;
    }

    // if failed - try default Adlib base 0x388
    if (esfm_try_detect(0x388)) {
        esfm_volume = 11;
        return 0x388;
    }

    // else return 0
    return 0;
}

