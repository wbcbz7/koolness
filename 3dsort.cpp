#include <algorith>
#include <vector>
#include <stdint.h>
#include "3dsort.h"

size_t zsort(std::vector<facelist_t> &flist, std::vector<zdata_t> &avgz) {    
    std::sort(avgz.begin(), avgz.end());
    std::vector<zdata_t>::iterator it;
    
    uint32_t rtn = avgz.begin()->idx;
    
    for (it = avgz.begin(); it < avgz.end(); it++) {
        flist[it->idx].next = &flist[(it+1)->idx];
    }
    flist[(it-1)->idx].next = NULL;
    
    return rtn;
}
