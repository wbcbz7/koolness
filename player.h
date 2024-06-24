#pragma once
#include <stdint.h>
#include "opmplay.h"

#define arrayof(n) (sizeof(n) / sizeof(n[0]))

enum {
    PLAY_MODE_LOOP = 0,
    PLAY_MODE_SHUFFLE,
    PLAY_MODE_SEQUENTIAL,
    PLAY_MODE_COUNT,
};

enum {
    TOTAL_TUNES = 9,
};

// module selector context
struct module_selector_entry_t {
    char *name;
    char *author;
    char *file;
    uint32_t loop_count;
    uint8_t *data;
    uint32_t raw_size;
    uint32_t unpacked_size;
};

struct module_selector_ctx_t {
    int cursor;
    int sel;
    int next_sel;
    int timeout;
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

    uint32_t            total_modules;
    bool                preload;
    bool                paused;
    bool                run;
    bool                volume_changed;
    int32_t             volume;

    // unpacked module for lowmem mode
    uint8_t*            unlz4_module;

    // VU meter stuff (later :)
    int                 bars[4 * 18];
    bar_keyofs_t        bars_keyofs[18];
    int                 bars_offset;
    int                 key[18];

    // fadeout
    struct {
        bool            run;
        int             timeout;
        int32_t         volume;
        int             loop_count;
        int32_t         timeout_mask;
    } nextsong;

    // play mode
    uint32_t            playmode;

    // text buffer
    uint16_t            textbuf[80 * 50];

    // selector ctx
    module_selector_ctx_t   selector;
};

// externs
extern player_ctx_t player_ctx;
extern module_selector_entry_t modulelist[TOTAL_TUNES];

bool player_init(bool preload);
void player_callback(void *parm);
void player_update_view();
int player_load_module_idx(player_ctx_t *ctx, int i);
int player_load_module(player_ctx_t *ctx, char *filename);
int player_load_module_packed(player_ctx_t *ctx, void *data, uint32_t size);

