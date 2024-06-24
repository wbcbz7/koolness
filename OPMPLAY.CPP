#include <stdint.h>
#include "opmplay.h"
#include "opmfile.h"
#include "esfmout.h"

// -------------------------
// file I/O procedures
static uint32_t opmplay_mem_read(opmplay_io_t* io, void* dst, uint32_t size) {
    if ((io == NULL) || dst == NULL) return OPMPLAY_ERR_NULLPTR;
    if ((size + io->offset) > io->size) return OPMPLAY_ERR_IO;
    opmplay_memcpy(dst, (uint8_t*)io->buf + io->offset, size);
    io->offset += size;
    return size;
}
static uint32_t opmplay_mem_seek(opmplay_io_t* io, uint32_t pos) {
    if (io == NULL) return OPMPLAY_ERR_NULLPTR;
    if (pos > io->size) return OPMPLAY_ERR_IO;
    io->offset = pos;
    return 0;
}

#ifdef OPMPLAY_ENABLE_STDIO
static uint32_t opmplay_file_read(opmplay_io_t* io, void* dst, uint32_t size) {
    return fread(dst, 1, size, io->io);
}
static uint32_t opmplay_file_seek(opmplay_io_t* io, uint32_t pos) {
    return fseek(io->io, pos, SEEK_SET);
}
#endif

// -------------------------

int opmplay_init(opmplay_context_t* ctx) {
    if (ctx == NULL) return OPMPLAY_ERR_NULLPTR;

    opmplay_memset(ctx, 0, sizeof(opmplay_context_t));

    return OPMPLAY_ERR_OK;
}

int opmplay_free(opmplay_context_t* ctx)
{
    if ((ctx->flags & OPMPLAY_MEMORY_IN_PLACE) == 0) {
        for (int ch = 0; ch < OPMPLAY_MAX_CHANNLES; ch++) {
            if (ctx->channels[ch].stream.data != NULL) {
                opmplay_memfree(ctx->channels[ch].stream.data); ctx->channels[ch].stream.data = NULL;
            }
        }
    }
    return OPMPLAY_ERR_OK;
}

// load file header
int opmplay_load_header(opmplay_context_t* ctx, opmplay_io_t* io) {
    if ((ctx == NULL) || (io == NULL)) return OPMPLAY_ERR_NULLPTR;

    // init file I/O handlers
    switch (io->type) {
#ifdef OPMPLAY_ENABLE_STDIO
    case OPMPLAY_IO_FILE:
        io->read = &opmplay_file_read;
        io->seek = &opmplay_file_seek;
        break;
#endif
    case OPMPLAY_IO_MEMORY:
    case OPMPLAY_IO_MEMORY_IN_PLACE:        // use some hacks :)
        io->read = &opmplay_mem_read;
        io->seek = &opmplay_mem_seek;
        break;
    case OPMPLAY_IO_USER:
        // use user-provided I/O functions
        if ((io->read == NULL) || (io->seek == NULL)) return OPMPLAY_ERR_BAD_PARAMETER;
        break;
    default:
        return OPMPLAY_ERR_BAD_PARAMETER;
    }

    ctx->flags = 0;
    if (io->type == OPMPLAY_IO_MEMORY_IN_PLACE) ctx->flags = OPMPLAY_MEMORY_IN_PLACE;

    // read header
    if (io->read(io, &ctx->header, sizeof(ctx->header)) != sizeof(ctx->header)) return OPMPLAY_ERR_IO;

    // and validate it
    if ((opmplay_memcmp(ctx->header.magic, "OPM\x1A", sizeof(ctx->header.magic))))
        return OPMPLAY_ERR_BAD_FILE_STRUCTURE;

    // done for now, waiting for opmplay_load_module :)
    return OPMPLAY_ERR_OK;
}

int opmplay_load_module(opmplay_context_t* ctx, opmplay_io_t* io) {
    if ((ctx == NULL) || (io == NULL)) return OPMPLAY_ERR_NULLPTR;
    uint32_t filepos = sizeof(opm_header_t);

    // allocate and copy stream data
    opm_header_stream_desc_t* streamdesc = (opm_header_stream_desc_t*)opmplay_alloc(sizeof(opm_header_stream_desc_t) * (OPMPLAY_MAX_CHANNLES));
    if (streamdesc == NULL) return OPMPLAY_ERR_NULLPTR;
    if (io->seek(io, filepos)) return OPMPLAY_ERR_IO;
    if (io->read(io, streamdesc, sizeof(opm_header_stream_desc_t) * (OPMPLAY_MAX_CHANNLES))
        != sizeof(opm_header_stream_desc_t) * (OPMPLAY_MAX_CHANNLES))
        return OPMPLAY_ERR_IO;

    // allocate and copy channel streams
    filepos += sizeof(opm_header_stream_desc_t) * (OPMPLAY_MAX_CHANNLES);
    for (int ch = 0; ch < OPMPLAY_MAX_CHANNLES; ch++) {
        if (ctx->flags & OPMPLAY_MEMORY_IN_PLACE) {
            ctx->channels[ch].stream.data = (uint8_t*)io->buf + filepos;
            filepos += streamdesc[ch].size;
        } else {
            ctx->channels[ch].stream.data = (uint8_t*)opmplay_alloc(sizeof(uint8_t*) * (streamdesc[ch].size));
            if (ctx->channels[ch].stream.data == NULL) return OPMPLAY_ERR_MEMALLOC;
            if (io->read(io, ctx->channels[ch].stream.data, streamdesc[ch].size) != streamdesc[ch].size) return OPMPLAY_ERR_IO;
        }
        ctx->channels[ch].stream.delay = 1;
    }

    // free stream descriptors
    opmplay_memfree(streamdesc);

    // rewind to start
    opmplay_rewind(ctx);

    // done :)
    return OPMPLAY_ERR_OK;
}

void opmplay_pop_stack(opmplay_channel_context_t* chctx) {
    opmplay_channel_stack_t* st = chctx->stack + (--chctx->stack_pos);
    chctx->stream.ptr = st->ptr;
    chctx->stream.samples_to_play = st->frames_to_play;
}

void opmplay_push_stack(opmplay_channel_context_t* chctx) {
    opmplay_channel_stack_t* st = chctx->stack + chctx->stack_pos;
    st->ptr = chctx->stream.ptr;
    st->frames_to_play = chctx->stream.samples_to_play;
    chctx->stack_pos++;
}

int opmplay_loop(opmplay_context_t* ctx) {
    // channel streams
    for (int ch = 0; ch < OPMPLAY_MAX_CHANNLES; ch++) {
        // init stack
        ctx->channels[ch].stack_pos = 0;
        ctx->channels[ch].stream.samples_to_play = -1;
        ctx->channels[ch].stream.ptr = ctx->channels[ch].stream.loop;
        ctx->channels[ch].stream.delay = ctx->channels[ch].stream.reload = 1;
    }
    ctx->pos.frame = ctx->pos.looped;

    return OPMPLAY_ERR_OK;
}

int opmplay_rewind(opmplay_context_t* ctx) {
    for (int ch = 0; ch < OPMPLAY_MAX_CHANNLES; ch++) {
        ctx->channels[ch].stream.loop = ctx->channels[ch].stream.data;
    }

    opmplay_loop(ctx);

    return OPMPLAY_ERR_OK;
}

// get and parse delay
static uint32_t opmplay_set_delay(uint8_t** data) {
    uint32_t delay = 0;
    if (**data == OPM_STREAM_DELAY_INT32) {
        delay = (
            (*(*data + 1) << 0) |
            (*(*data + 2) << 8) |
            (*(*data + 3) << 16) |
            (*(*data + 4) << 24)
            );
        *data += 5;
    }
    else if (**data == OPM_STREAM_DELAY_INT16) {
        delay = (
            (*(*data + 1) << 0) |
            (*(*data + 2) << 8)
            );
        *data += 3;
    }
    else if ((**data & 0xF0) == OPM_STREAM_DELAY_INT12) {
        delay = ((**data & 0x0F) << 8) | (*(*data + 1));
        *data += 2;
    }
    else if ((**data & 0xF0) == OPM_STERAM_DELAY_SHORT) {
        delay = (**data & 0xF) + 1;
        (*data)++;
    }
    return delay;
}

// update vu-meters, etc
static void opmplay_handle_write(opmplay_channel_context_t* chctx, int reg, int data) {
    chctx->view.regs.ch[reg]  = data;
    chctx->view.regmask      |= (1 << reg);
}

static int opmplay_preprocess_write(opmplay_context_t *ctx, int reg, int data) {
    switch(reg & 7) {
        case 1:
            // add volume offset for fadeout
            {
                int ksl = (data & 0xC0);
                int vol = (data & 0x3F) + ctx->view.vol_offset;
                if (vol > 0x3F) vol = 0x3F;
                data = vol | ksl;
            }
            break;
        case 7:
            // attenuate operator audio output by -6db
            // distortion fix for ES1869/Solo1?
            if ((data & 0xE0) > 0) data -= (2 << 5);
            break;
        default: break;
    }
    return data;
}

void opmplay_set_volume(opmplay_context_t *ctx, int32_t offset) {
    ctx->view.vol_offset = offset;

    int reg = 1;
    for (int ch = 0; ch < 18; ch++) {
        for (int op = 0; op < 4; op++) {
            int data = opmplay_preprocess_write(ctx, reg, ctx->channels[ch+1].view.regs.ch[reg & 31]);
            esfm_out(reg, data);
            reg += 8;
        }
    }
}

int opmplay_tick(opmplay_context_t* ctx) {

    int ch = 0;
    int rtn = OPMPLAY_ERR_OK;
    uint32_t newdelay = 0;

    // process control stream
    {
        opmplay_channel_context_t* chctx = ctx->channels + 0;
        uint8_t* data = chctx->stream.ptr;
        bool isRun = true;
        if (--chctx->stream.delay == 0) {
            while (isRun) {
                // check for common stuff
                switch (*data) {
                // end of stream - rewind everything
                case OPM_STREAM_END:
                    opmplay_loop(ctx);
                    isRun = false;
                    return OPMPLAY_ERR_END_OF_STREAM;
                    // just an NOP, break
                case OPM_STREAM_NEW_ORDER:
                case OPM_STREAM_NOP:
                    data++;
                    break;
                case OPM_STREAM_LOOP:
                    // save loop point
                    ctx->pos.looped = ctx->pos.frame;
                    chctx->stream.loop = data;
                    data++;
                    break;
                    // set new frame rate
                case OPM_STREAM_SET_FRAME_RATE:
                    ctx->header.frame_rate = *(uint16_t*)(data + 1); data += 3;
                    break;
                case OPM_STREAM_END_FRAME:
                    // end of frame - special case here
                    data++;
                    isRun = false;
                    break;

                default:
                    // test for delay
                    newdelay = opmplay_set_delay(&data);
                    if (newdelay > 0) {
                        chctx->stream.reload = newdelay;
                    }
                    else
                    {
                        return OPMPLAY_ERR_BAD_FILE_STRUCTURE;
                    }
                }
            }
            chctx->stream.delay = chctx->stream.reload;
            chctx->stream.ptr = data;
        }
    }

    // parse channel streams
    for (int ch = 0; ch < OPMPLAY_MAX_CHANNLES - 1; ch++) {
        opmplay_channel_context_t* chctx = ctx->channels + ch + 1;
        uint8_t* data = chctx->stream.ptr;
        bool isRun = true;
        if (--chctx->stream.delay == 0) {
            // reset view
            while (isRun) {
                // get streams
                if (*(data) < 32) {
                    int reg = (*data & 31) + (ch << 5);
                    int val = opmplay_preprocess_write(ctx, reg, *(data + 1));
                    data += 2;
                    esfm_out(reg, val);
                    opmplay_handle_write(chctx, reg & 31, val);
                    continue;
                }
                if ((*data & 0xFC) == OPM_KEY_TRIGGER) {
                    int key = *data & 1;
                    if (ch >= 16) {
                        esfm_out(0x240 + ((ch - 16) << 1) + 16 + 0, key);
                        esfm_out(0x240 + ((ch - 16) << 1) + 16 + 1, key);
                    }
                    else {
                        esfm_out(0x240 + ch, key);
                    }
                    if (*data & OPM_KEY_END_OF_FRAME)  isRun = false;
                    data++;

                    // update view
                    chctx->view.key = key | 2;
                    continue;
                }
                if ((*data & 0xF0) == OPM_STREAM_BACKREF) {
                    // back reference, nested call :)
                    int distance = ((*(data + 0) & 0x0F) << 8) | (*(data + 1));
                    int frames_to_play = *(data + 2);
                    chctx->stream.ptr = data + 3;
                    opmplay_push_stack(chctx);
                    data -= distance;
                    chctx->stream.samples_to_play = frames_to_play; // hack?
                    continue;
                }

                // check for common stuff
                switch (*data) {
                case OPM_STREAM_END:
                    // end of current stream, delay forever
                    chctx->stream.reload = -1;
                    isRun = false;
                    break;
                    // just an NOP, break
                case OPM_STREAM_NEW_ORDER:
                case OPM_STREAM_NOP:
                    data++;
                    break;
                case OPM_STREAM_LOOP:
                    // save loop point
                    chctx->stream.loop = data;
                    data++;
                    break;
                case OPM_STREAM_END_FRAME:
                    // end of frame - special case here
                    data++;
                    isRun = false;
                    break;

                default:
                    // test for delay
                    newdelay = opmplay_set_delay(&data);
                    if (newdelay > 0) {
                        chctx->stream.reload = newdelay;
                    }
                    else
                    {
                        return OPMPLAY_ERR_BAD_FILE_STRUCTURE;
                    }
                }
            }
            chctx->stream.delay = chctx->stream.reload;

            // decrement samples to play counter
            if (--chctx->stream.samples_to_play == 0) {
                // pop context from the stack
                do
                    opmplay_pop_stack(chctx);
                while (--chctx->stream.samples_to_play == 0);
            }
            else {
                // save data pointer
                chctx->stream.ptr = data;
            }
        }
    }

    ctx->pos.frame++;

    return rtn;
}

// get song length and loop point in frames
uint32_t opmplay_get_length(opmplay_context_t* ctx, uint32_t* loop_frames) {
    uint32_t total_frames = 0;

    bool end = false;
    int newdelay = 0;
    opmplay_channel_context_t* chctx = ctx->channels + 0;
    uint8_t* data = chctx->stream.ptr;
    chctx->stream.reload = 0;
    while (!end) {
        total_frames += chctx->stream.reload;
        bool isRun = true;
        while (isRun) {
            // check for common stuff
            switch (*data) {
                // end of stream - rewind everything
            case OPM_STREAM_END:
                isRun = false;
                end = true;
                break;
                // just an NOP, break
            case OPM_STREAM_NEW_ORDER:
            case OPM_STREAM_NOP:
                data++;
                break;
            case OPM_STREAM_LOOP:
                // save loop point
                if (loop_frames != NULL) *loop_frames = total_frames;
                data++;
                break;
                // set new frame rate
            case OPM_STREAM_SET_FRAME_RATE:
                data += 3;
                break;
            case OPM_STREAM_END_FRAME:
                // end of frame - special case here
                data++;
                isRun = false;
                break;

            default:
                // test for delay
                newdelay = opmplay_set_delay(&data);
                if (newdelay > 0) {
                    chctx->stream.reload = newdelay;
                }
                else
                {
                    return OPMPLAY_ERR_BAD_FILE_STRUCTURE;
                }
            }
        }
    }

    // rewind
    opmplay_rewind(ctx);
    return total_frames;
}
