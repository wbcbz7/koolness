#include "player.h"
#include "irq0.h"
#include "esfmout.h"
#include <flexptc.h>
#include <flexptc.h>
#include "main.h"
#include "unlz4.h"

player_ctx_t player_ctx;

// TODO: make it customizable!
module_selector_entry_t modulelist[TOTAL_TUNES] = {
    {"essneuro","","tunes/essneuro.lz4", 2},
    {"cielos esfumados", "",  "tunes/cielos.lz4", 1},
    {"deadline", "", "tunes/deadline.lz4", 2},
    {"devil detective", "", "tunes/DevilDet.lz4", 1},
    {"experiment", "", "tunes/expermnt.lz4", 1},
    {"second start", "", "tunes/2ndStart.lz4", 1},
    {"walk in the park", "", "tunes/WalkIn.lz4", 2},
    {"napalm loader rmx", "", "tunes/napalm.lz4", 1},
    {"x evil soul", "", "tunes/xevilsl.lz4", 2}
};

// unlz4 decompressor context
static unlz4_context_t unlz4_ctx;

int player_load_module_idx(player_ctx_t *ctx, int i) {
    return (ctx->preload ? player_load_module_packed(ctx, modulelist[i].data, modulelist[i].raw_size) : player_load_module(ctx, modulelist[i].file));
}

#if 0
// ------------------------
// load module from disk
int player_load_module(player_ctx_t *ctx, char *filename) {
    // pause playback
    ctx->paused = true;

    // stop current playback!
    opmplay_free(&ctx->opm);

    // clear bars
    memset(&ctx->bars, 0, sizeof(int)*4*18);

    // shut ESFM up
    esfm_reset();

#if 1
    // open file
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("can't open file!\n");
        return 1;
    }
#endif

    // load .opm file
    ctx->io.type = OPMPLAY_IO_FILE;
    ctx->io.io = f;
    ctx->paused = false;

    int rtn = 0;
    if ((rtn = opmplay_init(&ctx->opm)) != OPMPLAY_ERR_OK) {
        printf("unable to init ESFMPlay (error = %d)\n", rtn);
        return 1;
    }
    if ((rtn = opmplay_load_header(&ctx->opm, &ctx->io)) != OPMPLAY_ERR_OK) {
        printf("unable to load OPM header (error = %d)\n", rtn);
        return 1;
    };
    if ((rtn = opmplay_load_module(&ctx->opm, &ctx->io)) != OPMPLAY_ERR_OK) {
        printf("unable to load OPM module (error = %d)\n", rtn);
        return 1;
    };
    ctx->tempo_hz = ((0x1234DD00) / (ctx->opm.header.frame_rate)) << 8;
    ctx->frames_total = opmplay_get_length(&ctx->opm, &ctx->frames_loop);

#ifndef WIN32
    // install player proc
    if (mainprops.use_irq0) {
        irq0_setTimer(player_callback, ctx->opm.header.frame_rate, &player_ctx);
    } else {
        ptc_settimer(player_callback, player_ctx.tempo_hz >> 10, &player_ctx);
    }
#endif

    // close file
    fclose(f);

    // and reset pause
    ctx->paused = false;

    return 0;
}
#else 
// ------------------------
// load module from disk
int player_load_module(player_ctx_t *ctx, char *filename) {
    // pause playback
    ctx->paused = true;

    // stop current playback!
    opmplay_free(&ctx->opm);

    // clear bars
    memset(&ctx->bars, 0, sizeof(int)*4*18);

    // shut ESFM up
    esfm_reset();

    // deallocate current buffer
    if (ctx->unlz4_module != NULL) free(ctx->unlz4_module);

    FILE *f = fopen(filename, "rb");
    if (!f) return 1; else {
        unlz4_ctx.in = f;
        unlz4_ctx.available = 0;        // clear read buffer
        unlz4_ctx.pos = 0;
        unlz4_ctx.out_pos = 0;
        if (unlz4_decompress(&unlz4_ctx) == -1) {
            printf("unLZ4 error, aborting!\n");
            return true;
        }
        printf("done\n");
        ctx->unlz4_module = (uint8_t*)unlz4_ctx.out;
        fclose(f);
    }


    // load .opm file
    ctx->io.type = OPMPLAY_IO_MEMORY_IN_PLACE;
    ctx->io.buf = ctx->unlz4_module;
    ctx->io.offset = 0;
    ctx->io.size = unlz4_ctx.out_pos;
    ctx->paused = false;

    int rtn = 0;
    if ((rtn = opmplay_init(&ctx->opm)) != OPMPLAY_ERR_OK) {
        printf("unable to init ESFMPlay (error = %d)\n", rtn);
        return 1;
    }
    if ((rtn = opmplay_load_header(&ctx->opm, &ctx->io)) != OPMPLAY_ERR_OK) {
        printf("unable to load OPM header (error = %d)\n", rtn);
        return 1;
    };
    if ((rtn = opmplay_load_module(&ctx->opm, &ctx->io)) != OPMPLAY_ERR_OK) {
        printf("unable to load OPM module (error = %d)\n", rtn);
        return 1;
    };
    ctx->tempo_hz = ((0x1234DD00) / (ctx->opm.header.frame_rate)) << 8;
    ctx->frames_total = opmplay_get_length(&ctx->opm, &ctx->frames_loop);

#ifndef WIN32
    // install player proc
    if (mainprops.use_irq0) {
        irq0_setTimer(player_callback, ctx->opm.header.frame_rate, &player_ctx);
    } else {
        ptc_settimer(player_callback, player_ctx.tempo_hz >> 10, &player_ctx);
    }
#endif

    // reset pause
    ctx->paused = false;

    return 0;
}
#endif

// ------------------------
// load module from disk
int player_load_module_packed(player_ctx_t *ctx, void *data, uint32_t size) {
    // pause playback
    ctx->paused = true;

    // stop current playback!
    opmplay_free(&ctx->opm);

    // clear bars
    memset(&ctx->bars, 0, sizeof(int)*4*18);

    // shut ESFM up
    esfm_reset();

    // load .opm file
    ctx->io.type = OPMPLAY_IO_MEMORY_IN_PLACE;
    ctx->io.buf = data;
    ctx->io.offset = 0;
    ctx->io.size = size;
    ctx->paused = false;

    int rtn = 0;
    if ((rtn = opmplay_init(&ctx->opm)) != OPMPLAY_ERR_OK) {
        printf("unable to init ESFMPlay (error = %d)\n", rtn);
        return 1;
    }
    if ((rtn = opmplay_load_header(&ctx->opm, &ctx->io)) != OPMPLAY_ERR_OK) {
        printf("unable to load OPM header (error = %d)\n", rtn);
        return 1;
    };
    if ((rtn = opmplay_load_module(&ctx->opm, &ctx->io)) != OPMPLAY_ERR_OK) {
        printf("unable to load OPM module (error = %d)\n", rtn);
        return 1;
    };
    ctx->tempo_hz = ((0x1234DD00) / (ctx->opm.header.frame_rate)) << 8;
    ctx->frames_total = opmplay_get_length(&ctx->opm, &ctx->frames_loop);

#ifndef WIN32
    // install player proc
    if (mainprops.use_irq0) {
        irq0_setTimer(player_callback, ctx->opm.header.frame_rate, &player_ctx);
    } else {
        ptc_settimer(player_callback, player_ctx.tempo_hz >> 10, &player_ctx);
    }
#endif

    // reset pause
    ctx->paused = false;

    return 0;
}

bool player_init(bool preload) {
    // clear player context
    memset(&player_ctx, 0, sizeof(player_ctx));
    player_ctx.tempo_hz = (1 << 16);    // fix division by zero
    player_ctx.run = true;
    player_ctx.preload = preload;
    player_ctx.total_modules = TOTAL_TUNES;
    
    // preload all modules
    if (preload) {
        printf("loading music.."); fflush(stdout);
        for (int i = 0; i < player_ctx.total_modules; i++) {
            modulelist[i].data = NULL;

            //printf("loading %s...", modulelist[i].file); fflush(stdout);
            FILE *f = fopen(modulelist[i].file, "rb");
            if (!f) printf("can't load %s, skip\n", modulelist[i].file); else {
                unlz4_ctx.in = f;
                unlz4_ctx.available = 0;        // clear read buffer
                unlz4_ctx.pos = 0;
                unlz4_ctx.out_pos = 0;
                if (unlz4_decompress(&unlz4_ctx) == -1) {
                    printf("unLZ4 error, aborting!\n");
                    return true;
                }
                //printf("done\n");
                modulelist[i].data     = (uint8_t*)unlz4_ctx.out;
                modulelist[i].raw_size = unlz4_ctx.out_pos;
                fclose(f);
            }
            printf("."); fflush(stdout);
        }
    }
    printf("\n");

    return false;
}

// called once per frame
void player_update_view() {
#ifdef WIN32
    // manual step
    if (!player_ctx.paused) player_callback(&player_ctx.opm);
#endif
    // update bar info
    int barofs = 0;
    for (int ch = 0; ch < 18; ch++) {
        int regmask = 1 << 1;
        for (int op = 0; op < 4; op++) {
            if (player_ctx.opm.channels[ch + 1].view.regmask & regmask) {
                // update bars register
                player_ctx.bars[(ch << 2) + op] = 63 - (player_ctx.opm.channels[ch + 1].view.regs.op[op][1] & 0x3F);
                player_ctx.opm.channels[ch + 1].view.regmask &= ~regmask;
            }
            regmask <<= 8;
        }
        player_ctx.key[ch] = player_ctx.opm.channels[ch + 1].view.key;
        switch (player_ctx.key[ch] & 3) {
        case 3:
            // on
            player_ctx.bars_keyofs[ch].value = 16;
            player_ctx.bars_keyofs[ch].limit = 0;
            player_ctx.bars_keyofs[ch].dec   = 1;
            break;
        case 2:
            // off
            player_ctx.bars_keyofs[ch].limit = -64;
            player_ctx.bars_keyofs[ch].dec = 1;
            break;
        default: break;
        }
        player_ctx.opm.channels[ch + 1].view.key &= 1;
        if (player_ctx.bars_keyofs[ch].value > 0) barofs += player_ctx.bars_keyofs[ch].value;
    }
    barofs /= (18);
    player_ctx.bars_offset = player_ctx.bars_offset + (barofs - player_ctx.bars_offset)>>2;
}

// -------------------------
// player callback
void player_callback(void *parm) {
    player_ctx_t *ctx = (player_ctx_t*)parm;
    if (!ctx->paused) {
        // process module
        int rtn = opmplay_tick(&ctx->opm);

        // check for next song
        if ((rtn == OPMPLAY_ERR_END_OF_STREAM) && (ctx->playmode != PLAY_MODE_LOOP)) {
            if (--ctx->nextsong.loop_count <= 0) {
                switch(ctx->playmode) {
                    case PLAY_MODE_SEQUENTIAL:
                        ctx->selector.next_sel = (ctx->selector.sel + 1) % ctx->total_modules;
                        break;
                    case PLAY_MODE_SHUFFLE:
                        if (ctx->total_modules > 1) do {
                            ctx->selector.next_sel = rand() % ctx->total_modules;
                        } while (ctx->selector.next_sel == ctx->selector.sel);
                        break;
                    default: break;
                }
                ctx->nextsong.volume = ctx->volume;
                ctx->nextsong.timeout = 256;
                ctx->nextsong.timeout_mask = 15;
                ctx->nextsong.run = true;
            };
        }

        // check for fadeout
        if (ctx->nextsong.run) {
            ctx->nextsong.timeout--;
            if (((ctx->nextsong.timeout & ctx->nextsong.timeout_mask) == 0) && (ctx->nextsong.volume > 0))  {
                ctx->nextsong.volume--;
                esfm_set_volume(ctx->nextsong.volume);
            }
            if (ctx->nextsong.timeout == 0) {
                // switch to new module
                player_load_module_idx(ctx, ctx->selector.next_sel);
                player_ctx.nextsong.loop_count = modulelist[ctx->selector.next_sel].loop_count;
                ctx->selector.sel = ctx->selector.next_sel;
                esfm_set_volume(ctx->volume);
                ctx->nextsong.run = false;
            }
        }
    }

    // advance VU-meters
    bar_keyofs_t* bar_keyofs = ctx->bars_keyofs;
    for (int ch = 0; ch < 18; ch++) {
        bar_keyofs->value -= bar_keyofs->dec;
        if (bar_keyofs->value < bar_keyofs->limit) bar_keyofs->value = bar_keyofs->limit;
        bar_keyofs++;
    }
}



