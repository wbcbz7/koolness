#include <i86.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dos.h>
#include "esfmout.h"
#include "tinypci.h"
#include "main.h"

#include <flexptc.h>    // for vsync hack
#include "dpmi.h"
#include "lowlevel.h"   // for TSS exploit stuff
//#define VSYNC_HACK

uint32_t esfm_base;
static uint32_t esfm_volume_mixer;
static uint32_t esfm_volume = 15;

void esfm_delay(int timeout) {
    do { _asm {in al, 0xE1} } while (--timeout);
}

void esfm_set_baseport(uint32_t base) {
    esfm_base = base;
}
#if 1
#if 0
void esfm_out(int reg, int data) {
    _asm {
        mov     edx, [esfm_base]
        mov     eax, [reg]
        mov     ecx, [data]

        add     edx, 2
        out     dx, al
        in      al, 0xE1
        in      al, 0xE1
        mov     al, ah
        inc     edx
        out     dx, al
        in      al, 0xE1
        in      al, 0xE1
        mov     al, cl
        sub     edx, 2
        out     dx, al
        mov     edx, 0x3DA
        in      al, dx
        and     al, 8
        or      [ptc_force_vblank], al
    }
}
#endif
void esfm_out_fast(int reg, int data) {
    _asm {
        mov     edx, [esfm_base]
        mov     eax, [reg]
        mov     ecx, [data]

        add     edx, 2
        out     dx, al
        in      al, 0xE1
        mov     al, ah
        inc     edx
        out     dx, al
        in      al, 0xE1
        mov     al, cl
        sub     edx, 2
        out     dx, al
        mov     edx, 0x3DA
        in      al, dx
        and     al, 8
        or      [ptc_force_vblank], al
    }
}
#else
void esfm_out(int reg, int data) {
    outp(esfm_base + 2, (reg & 0xFF));
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
    outp(esfm_base + 3, (reg >> 8));
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
    outp(esfm_base + 1, data);
#ifdef VSYNC_HACK
    if (inp(0x3da) & 8) ptc_force_vblank = true;
#else
    _asm {in al, 0xE1};
    _asm {in al, 0xE1};
#endif
}
#endif

void esfm_out_slow(int reg, int data) {
    outp(esfm_base + 2, (reg & 0xFF));
    esfm_delay(4);
    outp(esfm_base + 3, (reg >> 8));
    esfm_delay(4);
    outp(esfm_base + 1, data);
    esfm_delay(4);
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

    // restore mixer volume
    esfm_set_volume(esfm_volume_mixer);
}

void esfm_reset() {
    // TODO: confirm
    for (int i = 0; i < 0x7FF; i++) {
        esfm_out_slow(i, 0);
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
    if (mainprops.disable_volume_ctrl) return;
    if (esfm_base != 0x388) {
        volume &= 15;
        outp(esfm_base + 0x04, 54); esfm_delay(1);
        outp(esfm_base + 0x05, (volume << 4) | (volume)); esfm_delay(1);
    }
    esfm_volume = volume;
}

uint32_t esfm_get_volume() {
    if (mainprops.disable_volume_ctrl) return 15;
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

// ----------------------
// evil TSS hack to gain access to ESFM ports under Win9x WDM drivers :meatjob:

static uint8_t iopm_saved[2]; // saved IO permission map for 16 consecutive ports

struct tss_info_t {
    uint8_t* base;
    uint32_t limit;         // limit+1
    uint32_t ioperm_ofs;    // offset of I/O permission map
    uint32_t ioperm_size;
};

static int get_tss(tss_info_t * tss) {
    if (tss == NULL) return 1;

    descriptorTableAddr gdtPtr;
    sgdt(&gdtPtr);

    _dpmi_descriptor* gdt = (_dpmi_descriptor*)gdtPtr.base;
    _dpmi_descriptor* cdt = gdt;      // current descriptor table
    uint32_t cdt_limit = gdtPtr.limit;
    uint32_t tssSelector = str();

    if (tssSelector != 0) {
        // found it
        if (tssSelector & 4) {
            // TSS located in LDT
            uint32_t ld = sldt(); _dpmi_descriptor* ldt = (_dpmi_descriptor*)(FP_OFF(gdt) + (ld & ~7));
            void* ldtAddr = (void*)((ldt->base_low) | (ldt->base_mid << 16) | (ldt->base_high << 24));
            cdt = (_dpmi_descriptor*)ldtAddr;
            cdt_limit = ((ldt->limit_low) | (ldt->limit_high << 16));    // TODO: limit granularity check?
        }

        // check if TSS is within limits (technically this should never happen as CPU will fault if invalid TSS is loaded)
        if (tssSelector > cdt_limit) return 1;

        // find TSS in current descriptor table
        _dpmi_descriptor *tssDescriptor = cdt + (tssSelector >> 3);
        tss->base        = (uint8_t*)((tssDescriptor->base_low) | (tssDescriptor->base_mid << 16) | (tssDescriptor->base_high << 24));
        tss->limit       = ((tssDescriptor->limit_low) | (tssDescriptor->limit_high << 16));    // TODO: limit granularity check?
        tss->ioperm_ofs  = *(uint16_t*)(tss->base + 0x66);
        if (tss->ioperm_ofs > tss->limit) return 1; // ioperm map is invalid
        tss->ioperm_size = (tss->limit+1) - tss->ioperm_ofs;

        return 0;
    } else return 1;
}

int flip_iopm(uint32_t base, uint32_t num_ports, bool unlock) {
    // first check if we're on ring0 already, then we don't need to mess with this stuff at all
    if ((CS() & 3) == 0) return 0;

    // if num_ports higher than 16, fail (not enough room in static storage for saved IOPM)
    if (num_ports > 16) return 1;

    // get TSS and check if there's enough room for registers to unlock
    tss_info_t tssinfo;
    if ((get_tss(&tssinfo)) || (tssinfo.base == NULL)) return 1;   // oops no TSS

    // adjust base port and ports num
    base >>= 3; num_ports >>= 3;
    if ((base + num_ports - 1) >= tssinfo.ioperm_size) return 1;      // not covered by TSS, can't patch

    if (unlock) {
        for (int i = 0; i < num_ports; i++) {
            iopm_saved[i] = tssinfo.base[tssinfo.ioperm_ofs + base + i];
            tssinfo.base[tssinfo.ioperm_ofs + base + i] = 0;    // all ports unlocked!
        }
    } else {
        for (int i = 0; i < num_ports; i++) {
            tssinfo.base[tssinfo.ioperm_ofs + base + i] = iopm_saved[i];
        }
    }

    // reload TSS by triggering ring0 switch
    _asm {
        mov eax, 0x0400
        int 0x31            // get DPMI version, discard results
    }

    return 0;
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
#ifdef DEBUG
    printf("iobase = %X\n", iobase);
#endif

    // unlock ports in IOPM if requested
    if (mainprops.tss_unlock)     flip_iopm(iobase, 16, true);

    // activate ESFM
    if (mainprops.activate_mixer) esfm_activate(iobase);

    // try detect here
    if (esfm_try_detect(iobase)) {
        // found!
        esfm_base = iobase;
        // unmute if muted
        // TODO: detect dosbox-x somehow (it's too quiet now!)
        esfm_volume_mixer = esfm_get_volume();
        esfm_set_volume(15);
        return iobase;
    }

    // if failed - try default Adlib base 0x388
    if (esfm_try_detect(0x388)) {
        esfm_base = 0x388;
        esfm_volume = 15;
        return 0x388;
    }

    // else return 0
    return 0;
}

