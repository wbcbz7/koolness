#pragma once

#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

// call it before any rtc_setTimer() calls
void rtc_initTimer(int divisor);

// call it before exit or you will get...guess what? :)
void rtc_freeTimer();

// replaces current timer
// delta = (func_call_rate_in_hz << 16) / (timer_tick_rate_in_hz)
void rtc_setTimer(void (*func)(void*), uint32_t delta, void* parm);

// get global tick count
uint32_t rtc_getTick();

// set global tick count
void rtc_setTick(uint32_t tick);

// get timer rate in Hz
uint32_t rtc_getTimerRate();

#ifdef _cplusplus
}
#endif
