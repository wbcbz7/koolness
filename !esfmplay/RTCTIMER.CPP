#include <i86.h>
#include <dos.h>
#include <stdint.h>
#include <stdlib.h>
#include <conio.h>

#include "rtctimer.h"

/*

    RTC timer procedures v.1.01
    by wbcbz7 12.11.2o15 - 23.o3.2o16 - 18.o4.2o24

    some guidelines :)
    - timer can run at freqs from 2 to 8192 hz (look at rtc_timeRate and
    rtc_clockDivisor defines)
    - your timer proc should avoid any calls to some stdlib functions and
    bios and dos interrupts bcoz it can lead to crash
    - works in real and protected mode under pure dos
    - works under windows (at least under mustdie'98 :) but 8192hz isn't stable

    conversion table:
    rtc_timerRate | rtc_clockDivisor
        1024      |        6
        2048      |        5
        4096      |        4
        8192      |        3
    I guess nobody will use freqs below 1024hz ;), but if you want to use them
    look at this formula:

    rtc_timerRate = (32768 >> (rtc_clockDivisor - 1)); rtc_timerRate <= 8192 (!)

    known issues:
    - via vt82855n - when using 8192hz rate timer can stop after 10-15 seconds
    while 1024hz works fine

    changelog:
    v.1.03  - use delta increment instead of down count, minor fixes
    v.1.02  - c++ compatible :)
    v.1.01  - added manual divisor selection in rtc_InitTimer()
    v.1.00  - initial release (used in blash\bz7)

*/

#ifdef _cplusplus
extern "C" {
#endif

static uint32_t rtc_timerRate    = 1024;  // default
static uint32_t rtc_clockDivisor = 6;

static void (*rtc_timerProc)(void* parm) = NULL;    // timer procedure
static void *rtc_timerProcParm = NULL;              // timer procedure parameter
static void (__interrupt __far *rtc_oldhandler)();  // internal procedure - old INT70h handler

static int rtc_divisor;

#define rtc_stack 65536 // шоб наверняка :)

// stack stuff
static uint8_t *rtc_stackbuf;
static uint8_t __far *rtc_stacktop;
static uint32_t rtc_stacksize;
static uint8_t  rtc_stackused;
static void __far *rtc_oldssesp;

void stackcall(void *, void *);
#pragma aux stackcall parm [edx] [eax] = \
  "mov word ptr rtc_oldssesp+4,ss" \
  "mov dword ptr rtc_oldssesp+0,esp" \
  "lss esp,rtc_stacktop" \
  "sti" \
  "call edx" \
  "cli" \
  "lss esp,rtc_oldssesp"

void loades();
#pragma aux loades = "push ds" "pop es"

static volatile uint32_t rtc_tick;
static volatile uint32_t rtc_delta_acc, rtc_delta;           // 16.16fx

void __interrupt __far rtc_handler() {
    loades();
    _disable();
    rtc_tick++;                                     // increment global RTC IRQ counter
    do outp(0x70, 0xC); while (inp(0x71) & 0x80);   // clear interrupt flags in RTC
    outp(0xA0, 0x20); outp(0x20, 0x20);             // and send EOI to interrupt controller

    // check if to call external procedure
    if (rtc_delta_acc >= (1 << 16)) {
        rtc_delta_acc -= (1 << 16);
        if ((rtc_timerProc != NULL) && (!rtc_stackused)) {
            rtc_stackused++;
            _enable();
            stackcall(rtc_timerProc, rtc_timerProcParm);
            _disable();
            rtc_stackused--;
        }
    }

    // incerement counter
    rtc_delta_acc += rtc_delta;
    _enable();
}

// call it before any rtc_setTimer() calls
void rtc_initTimer(int divisor) {
    rtc_clockDivisor = divisor & 0xF;
    rtc_timerRate = (32768 >> (rtc_clockDivisor - 1));

    // stack init
    rtc_stackbuf = new uint8_t[rtc_stack];
    if (!rtc_stackbuf) return;
    rtc_stacktop = (uint8_t _far *)(rtc_stackbuf + rtc_stack);
    rtc_stackused = 0;

    _disable();
    rtc_oldhandler = _dos_getvect(0x70);
    _dos_setvect(0x70, rtc_handler);
    outp(0xA1, (inp(0xA1) & 0xFE));                                        // unmask IRQ8
    outp(0x70, 0x8A); outp(0x71, ((inp(0x71) & 0xF0) | rtc_clockDivisor)); // disable NMI and select rate
    outp(0x70, 0x8B); outp(0x71,  (inp(0x71) | 0x40));                     // enable periodic interrupt
    outp(0x70, 0xD);  inp(0x71);                                           // enable NMI

    rtc_tick = 0;
    _enable();
}

// call it before exit or you will get...guess that? :)
void rtc_freeTimer() {
    _disable();
    _dos_setvect(0x70, rtc_oldhandler);
    // we will not mask IRQ8 because we will return control to BIOS interrupt handler
    outp(0x70, 0x8A); outp(0x71, ((inp(0x71) & 0xF0) | 0x6)); // disable NMI and select 1024 hz rate
    outp(0x70, 0x8B); outp(0x71,  (inp(0x71) & 0xBF));        // disable periodic interrupt
    outp(0x70, 0xD);  inp(0x71);                              // enable NMI
    _enable();

    delete[] rtc_stackbuf;
}

// replaces current timer
void rtc_setTimer(void (*func)(void*), uint32_t delta, void* parm) {
    _disable();
    rtc_timerProc = func;
    rtc_delta = delta;
    rtc_timerProcParm = parm;
    rtc_delta_acc = 0;
    _enable();
}

uint32_t rtc_getTick() {
    return rtc_tick;
}

void rtc_setTick(uint32_t tick) {
    rtc_tick = tick;
}

uint32_t rtc_getTimerRate() {
    return rtc_timerRate;
}

#ifdef _cplusplus
}
#endif
