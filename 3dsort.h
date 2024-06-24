#pragma once

#include <stdint.h>
#include <vector>

#include "poly.h"

struct zdata_t {
    uint32_t    idx;
    int32_t     avgz;
    
    bool operator<(const zdata_t &a) const {
        return avgz > a.avgz;
    }
};

// perform polygon sorting via list linking

uint32_t zsort(std::vector<facelist_t> &flist, std::vector<zdata_t> &avgz);
