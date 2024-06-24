#pragma once
#include <stdint.h>
#include "opmfile.h"

// OPMPlay setup defines
#define OPMPLAY_ENABLE_STDIO

#ifdef OPMPLAY_ENABLE_STDIO
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#endif

// LXMPlay import defines
#define opmplay_memcpy  memcpy
#define opmplay_memset  memset
#define opmplay_memcmp  memcmp
#define opmplay_alloc   malloc
#define opmplay_memfree free
#define opmplay_debug_printf(...)  printf(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

// general enums
enum {
    OPMPLAY_MAX_CHANNLES            = 18+1,
    OPMPLAY_MAX_STACK_DEPTH         = 4,
};

// return error codes
enum {
    OPMPLAY_ERR_OK                  = 0,
    OPMPLAY_ERR_END_OF_STREAM       = 1,
    OPMPLAY_ERR_BAD_FILE_STRUCTURE  = -1,
    OPMPLAY_ERR_MEMALLOC            = -2,
    OPMPLAY_ERR_NULLPTR             = -3,
    OPMPLAY_ERR_NO_SOUNDRAM         = -4,
    OPMPLAY_ERR_DEVICE              = -5,
    OPMPLAY_ERR_BAD_PARAMETER       = -6,
    OPMPLAY_ERR_IO                  = -7,


    OPMPLAY_ERR_LOOP                = -8,       // actually not an error!

    OPMPLAY_ERR_UNKNOWN             = -20,
};

enum {
    OPMPLAY_IO_USER                 = 0,
    OPMPLAY_IO_FILE                 = 1,
    OPMPLAY_IO_MEMORY               = 2,
    OPMPLAY_IO_MEMORY_IN_PLACE      = 3,
};

enum {
    OPMPLAY_MEMORY_IN_PLACE         = (1 << 1),
};

// file I/O structs
struct opmplay_io_t {
    uint32_t    type;          // i/o type
    union {
        void* buf;
#ifdef OPMPLAY_ENABLE_STDIO
        FILE* io;
#endif
    };
    uint32_t    size;

    // internal
    uint32_t    offset;
    uint32_t(*read)(opmplay_io_t* io, void* dst, uint32_t size);    // returns bytes read
    uint32_t(*seek)(opmplay_io_t* io, uint32_t offset);             // returns 0 if success
};

struct opmplay_channel_stack_t {
    uint8_t* ptr;
    uint32_t frames_to_play;
};

struct opmplay_channel_context_t {
    // stack 
    opmplay_channel_stack_t stack[OPMPLAY_MAX_STACK_DEPTH];
    uint32_t                stack_pos;

    // stream data
    struct {
        uint32_t            samples_to_play;
        uint32_t            delay;
        uint32_t            reload;

        uint8_t*            data;
        uint8_t*            ptr;
        uint8_t*            loop; // if active
    } stream;

    // internal registers
    uint8_t block;          // used to track key on/off changes

    // view
    struct view {
        union {
           uint8_t op[4][8];
           uint8_t ch[32];
        } regs;
        uint32_t regmask;
        uint8_t  key;
    } view;
};

struct opmplay_context_t {
    // LXM file header
    opm_header_t                header;

    // channel context
    opmplay_channel_context_t   channels[1+18];

    // position data 
    struct {
        uint32_t            order;
        uint32_t            frame;
        uint32_t            samples;

        uint32_t            looped;
    } pos;

    // misc flags
    uint32_t                flags;

    // view
    struct {
        int32_t             vol_offset;
    } view;

};


// init context
int opmplay_init(opmplay_context_t* ctx);

// free context
int opmplay_free(opmplay_context_t* ctx);

// load file header
int opmplay_load_header(opmplay_context_t* ctx, opmplay_io_t* io);

// load file contents
int opmplay_load_module(opmplay_context_t* ctx, opmplay_io_t* io);

// reset to start
int opmplay_rewind(opmplay_context_t* ctx);

// play one tick, write to ESFM chip
int opmplay_tick(opmplay_context_t* ctx);

// get song length in frames and loop point
uint32_t opmplay_get_length(opmplay_context_t* ctx, uint32_t* loop_frames);

// set volume offset (0 - full, 64 - off)
void opmplay_set_volume(opmplay_context_t *ctx, int32_t offset);

#ifdef __cplusplus
}
#endif
