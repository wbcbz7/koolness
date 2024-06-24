#include <i86.h>
#include <dos.h>
#include <stdint.h>
#include <stdlib.h>
#include <conio.h>

#include "irq0.h"

/*

    IRQ0 timer (hack lol)

*/

#ifdef _cplusplus
extern "C" {
#endif

static void (*irq0_timerProc)(void* parm) = NULL;    // timer procedure
static void *irq0_timerProcParm = NULL;              // timer procedure parameter
static void (__interrupt __far *irq0_oldhandler)();  // internal procedure - old INT70h handler

#define irq0_stack 65536 // шоб наверняка :)

// stack stuff
static uint8_t *irq0_stackbuf;
static uint8_t __far *irq0_stacktop;
static uint32_t irq0_stacksize;
static uint8_t  irq0_stackused;
static void __far *irq0_oldssesp;

void stackcall(void *, void *);
#pragma aux stackcall parm [edx] [eax] = \
  "mov word ptr irq0_oldssesp+4,ss" \
  "mov dword ptr irq0_oldssesp+0,esp" \
  "lss esp,irq0_stacktop" \
  "sti" \
  "call edx" \
  "cli" \
  "lss esp,irq0_oldssesp"

void loades();
#pragma aux loades = "push ds" "pop es"

static volatile uint32_t irq0_tick;
static volatile uint32_t irq0_delta_acc, irq0_delta;           // 16.16fx

void __interrupt __far irq0_handler() {
    loades();
    _disable();
    irq0_tick++;                                     // increment global RTC IRQ counter    

    // first check if to call old IRQ0 handler

    // check if to call external procedure
    if (irq0_delta_acc >= (1 << 16)) {
        irq0_delta_acc -= (1 << 16);
        irq0_oldhandler();          // should send EOI by itself
    } else outp(0x20, 0x20);        // else we send EOI

    // call user handler
    if ((irq0_timerProc != NULL) && (!irq0_stackused)) {
        irq0_stackused++;
        _enable();
        stackcall(irq0_timerProc, irq0_timerProcParm);
        _disable();
        irq0_stackused--;       
    }

    // increment counter
    irq0_delta_acc += irq0_delta;
    _enable();
}

// call it before any irq0_setTimer() calls
void irq0_initTimer() {
    // stack init
    irq0_stackbuf = new uint8_t[irq0_stack];
    if (!irq0_stackbuf) return;
    irq0_stacktop = (uint8_t _far *)(irq0_stackbuf + irq0_stack);
    irq0_stackused = 0;

    _disable();
    irq0_oldhandler = _dos_getvect(0x8);
    // set 18.2hz rate
    outp(0x43, 0x34); outp(0x40, 0); outp(0x40, 0);
    irq0_timerProc = 0;
    irq0_delta = 0x10000;
    irq0_delta_acc = 0;
    _dos_setvect(0x8, irq0_handler);

    irq0_tick = 0;
    _enable();
}

// call it before exit or you will get...guess that? :)
void irq0_freeTimer() {
    _disable();
    _dos_setvect(0x08, irq0_oldhandler);
    // set 18.2hz rate
    outp(0x43, 0x36); outp(0x40, 0); outp(0x40, 0);
    _enable();

    delete[] irq0_stackbuf;
}

// replaces current timer
void irq0_setTimer(void (*func)(void*), uint32_t delta, void* parm) {
    _disable();
    irq0_timerProc = func;
    irq0_delta = delta;
    irq0_timerProcParm = parm;
    // set new rate
    outp(0x43, 0x34); outp(0x40, (delta & 0xFF)); outp(0x40, (delta >> 8));
    _enable();
}

uint32_t irq0_getTick() {
    return irq0_tick;
}

void irq0_setTick(uint32_t tick) {
    irq0_tick = tick;
}

#ifdef _cplusplus
}
#endif
