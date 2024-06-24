#pragma once
#include <stdint.h>

enum {
    CMD_FLAG_NONE,
    CMD_FLAG_BOOL,
    CMD_FLAG_INT,
    CMD_FLAG_HEX,
    CMD_FLAG_STRING,

    CMD_FLAG_MASK = (1 << 16) - 1,
    CMD_FLAG_MULTI = (1 << 20),
};

struct cmdline_t {
    const char  shortname;
    char        flags;
    const char* longname;
    void*       parmPtr;
    uint32_t    parmLength;     // string or binary data only
};

// parse command line
uint32_t parse_cmdline(int argc, char* argv[], const cmdline_t* params, uint32_t paramCount, int startParam = 1);
