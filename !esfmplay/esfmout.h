#pragma once

#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

void esfm_delay(int timeout);
void esfm_set_baseport(uint32_t base);
void esfm_out(int reg, int data);
void esfm_enable();
void esfm_disable();
void esfm_reset();
uint32_t esfm_detect();

void esfm_set_volume(uint32_t volume);
uint32_t esfm_get_volume();

#ifdef _cplusplus
}
#endif
