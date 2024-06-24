#pragma once
#include <stdint.h>

union argb32 {
    struct {
        uint8_t b, g, r, a;
    };
    uint32_t v;
};

