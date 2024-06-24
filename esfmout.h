#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t esfm_base;

#ifdef WIN32

#define esfm_delay(...)
#define esfm_set_baseport(...)
#define esfm_out(...)
#define esfm_enable(...)
#define esfm_disable(...)
#define esfm_reset(...)
#define esfm_detect(...)
#define esfm_set_volume(...)
#define esfm_get_volume(...)

#else
void esfm_delay(int timeout);
void esfm_set_baseport(uint32_t base);
void esfm_out(int reg, int data);
#pragma aux esfm_out parm [eax] [ecx] modify [eax ecx edx]
void esfm_out_slow(int reg, int data);
#pragma aux esfm_out parm [eax] [ecx] modify [eax ecx edx]
void esfm_enable();
void esfm_disable();
void esfm_reset();
uint32_t esfm_detect();

void esfm_set_volume(uint32_t volume);
uint32_t esfm_get_volume();
#endif

#ifdef __cplusplus
}
#endif


