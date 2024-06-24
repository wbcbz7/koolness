#pragma once

#include "vec.h"
#include "poly.h"

// init tmapper
void tmap_init(void *buf, int xres, int yres, int pitch);

// draw flat face
void facedraw_flat(facelist_t *f);

