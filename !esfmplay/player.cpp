#include "player.h"
#include "irq0.h"
#include "esfmout.h"

player_ctx_t player_ctx;

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

    // open file
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("can't open file!\n");
        return 1;
    }

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

    // install player proc
    irq0_setTimer(player_callback, ctx->opm.header.frame_rate, &player_ctx);

    // close file
    fclose(f);

    // and reset pause
    ctx->paused = false;

    return 0;
}

void player_init() {
    // clear player context
    memset(&player_ctx, 0, sizeof(player_ctx));
    player_ctx.tempo_hz = (1 << 16);    // fix division by zero
    player_ctx.run = true;
}

// called once per frame
void player_update_view() {
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
    if (!ctx->paused) opmplay_tick(&ctx->opm);

    // advance VU-meters
    bar_keyofs_t* bar_keyofs = ctx->bars_keyofs;
    for (int ch = 0; ch < 18; ch++) {
        bar_keyofs->value -= bar_keyofs->dec;
        if (bar_keyofs->value < bar_keyofs->limit) bar_keyofs->value = bar_keyofs->limit;
        bar_keyofs++;
    }
    if (ctx->volume_changed) {
        ctx->volume_changed = false;
        opmplay_set_volume(&ctx->opm, ctx->volume);
    }
}



