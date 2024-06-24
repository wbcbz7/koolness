#pragma once

#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

// call it before any irq_setTimer() calls
void irq0_initTimer();

// call it before exit or you will get...guess what? :)
void irq0_freeTimer();

// replaces current timer
// delta = 0x1234DD / timer_tick_rate_in_hz
void irq0_setTimer(void (*func)(void*), uint32_t delta, void* parm);

// get global tick count
uint32_t irq0_getTick();

// set global tick count
void irq0_setTick(uint32_t tick);

#ifdef _cplusplus
}
#endif
