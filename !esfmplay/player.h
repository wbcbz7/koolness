#pragma once
#include <stdint.h>
#include "opmplay.h"

#define arrayof(n) (sizeof(n) / sizeof(n[0]))

// module selector context
struct module_selector_entry_t {
    char *name;
    char *file;
};

struct module_selector_ctx_t {
    int cursor;
    int sel;
};

// player context
struct bar_keyofs_t {
    int value;
    int limit;
    int dec;
};

struct player_ctx_t {
    opmplay_context_t   opm;
    opmplay_io_t        io;
    uint32_t            tempo_hz;
    uint32_t            frames_total;
    uint32_t            frames_loop;

    bool                paused;
    bool                run;
    bool                volume_changed;
    int32_t             volume;

    // VU meter stuff (later :)
    int                 bars[4 * 18];
    bar_keyofs_t        bars_keyofs[18];
    int                 bars_offset;
    int                 key[18];

    // text buffer
    uint16_t            textbuf[80 * 50];

    // selector ctx
    module_selector_ctx_t   selector;
};

// externs
extern player_ctx_t player_ctx;
extern module_selector_entry_t modulelist[6];

void player_init();
void player_callback(void *parm);
void player_update_view();
int player_load_module(player_ctx_t *ctx, char *filename);

