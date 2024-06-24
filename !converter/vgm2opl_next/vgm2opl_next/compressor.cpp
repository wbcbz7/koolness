#include <stdint.h>
#include "opmctx.h"

enum {
    MAX_BACKREF_DISTANCE_FRAMES = 255,
    MAX_BACKREF_DISTANCE_BYTES = 4095,            // HACK HACK HACK!!!!!
    MAX_BACKREF_LENGTH_FRAMES = 255,
};

// -------------------------------------
// compressor!

struct match_info_t {
    int logic_frames;       // nested backrefs count as 1 frame
    int total_frames;       // incl. count in nested backrefs
    int distance_frames;    // distance in frames
    int distance_bytes;     // distance or length in bytes
};

// test current match, returns length or 0 if not matched, can be recursive
static match_info_t test_match(
    std::vector<opm_channel_record_t>& srcdata, std::vector<opm_channel_record_t>& window,
    int datapos, int windowpos, int depth, int max_length = INT32_MAX
) {
    match_info_t rtn = { 0 };
    if ((depth == 0) || (max_length == 0)) return rtn;

    // start scanning from pos
    int srcpos = datapos;
    int dstpos = windowpos;
    do {
        auto src = srcdata.begin() + srcpos;
        auto dst = window.begin() + dstpos;
        // check for match
        if ((src == srcdata.end()) || (dst == window.end())) break;
        if (dst->flags & OPM_CHAN_BACKREF) {
            // do recursive match
            auto nested_match = test_match(srcdata, window, srcpos, dstpos - dst->distance_frames, depth - 1, dst->frames_to_play);
            if ((dst->frames_to_play == 0) || (nested_match.logic_frames != dst->frames_to_play)) break; else {
                // add this backref to match count
                dstpos++;
                srcpos += nested_match.logic_frames;
                rtn.logic_frames++;
                rtn.total_frames += nested_match.logic_frames;
            }
        }
        else {
            // no backref, scan current frame
            if ((src->rawdata != dst->rawdata) || (src->frame_dist != dst->frame_dist)) {
                break;
            } else if (rtn.distance_bytes + src->rawdata.size() >= MAX_BACKREF_DISTANCE_BYTES)
            {
                // match failure or too long - enough for us :)
                break;
            } else {
                // match! increment everything
                srcpos++; dstpos++;
                rtn.logic_frames++;
                rtn.total_frames++;
                rtn.distance_bytes += src->rawdata.size();
            }
        }
    } while (--max_length != 0);

    // validate
    if ((rtn.distance_bytes >= MAX_BACKREF_DISTANCE_BYTES) ||
        (rtn.logic_frames >= MAX_BACKREF_DISTANCE_FRAMES))
        return { 0 };

    return rtn;
}

// find match
static match_info_t find_match(opm_convert_context_t *ctx,
    std::vector<opm_channel_record_t>& srcdata, std::vector<opm_channel_record_t>& dstdata,
    int current_pos, int max_lookback, int min_backref_length
) {
    match_info_t ret = { 0 };

    // start from back
    int start = dstdata.size() - max_lookback; if (start < 0) start = 0;
    int stop = srcdata.size();

    for (int pos = start; pos < dstdata.size(); pos++) {
        int srcpos = current_pos;
        int dstpos = pos;

        // check if max lookback in bytes is not violated
        if ((srcdata[srcpos].byte_stamp - dstdata[dstpos].byte_stamp) > MAX_BACKREF_DISTANCE_BYTES)
            continue;

        // test current match
        auto match = test_match(srcdata, dstdata, srcpos, dstpos, ctx->flags.max_stack_depth, MAX_BACKREF_LENGTH_FRAMES);

        // check if match is found
        if (match.total_frames >= min_backref_length) {
            // emit match and end
            ret.distance_frames = dstdata.size() - pos;
            ret.logic_frames = match.logic_frames;
            ret.total_frames = match.total_frames;
            ret.distance_bytes = match.distance_bytes;
            return ret;
        }
    }

    return {};
}

uint32_t opm_compress_channel(opm_convert_context_t* ctx, int ch, int min_backref_length) {
    ctx->opmpacked[ch].clear();
    auto& src_ch = ctx->opmrecords[ch];
    int pos = 0; uint32_t total_bytes = 0;
    while (pos != src_ch.size()) {
        // find match
        auto match = find_match(ctx, src_ch, ctx->opmpacked[ch], pos, MAX_BACKREF_DISTANCE_FRAMES, min_backref_length);
        if (match.logic_frames == 0) {
            // copy one frame, recalculate byte stamp!
            opm_channel_record_t rec = src_ch[pos];
            rec.byte_stamp = total_bytes;
            ctx->opmpacked[ch].push_back(src_ch[pos]);
            total_bytes += src_ch[pos].rawdata.size();
            pos++;
        }
        else {
            // set back reference - copy main parameters from original
            opm_channel_record_t rec = src_ch[pos];
            rec.byte_stamp = total_bytes;
            rec.flags |= OPM_CHAN_BACKREF;
            rec.distance_frames = match.distance_frames;
            rec.frames_to_play = match.logic_frames;
            // fill dummy rawdata (will resolve this later!)
            rec.rawdata.clear();
            rec.rawdata.push_back(OPM_STREAM_BACKREF);
            rec.rawdata.push_back(0);
            rec.rawdata.push_back(rec.frames_to_play);
            ctx->opmpacked[ch].push_back(rec);
            pos += match.total_frames; // skip matched data
            total_bytes += rec.rawdata.size();
        }
    }
    if (ctx->flags.verbosity >= 2) {
        printf("ch %d min backref %d - %d records, %d bytes\n", ch, min_backref_length, ctx->opmpacked[ch].size(), total_bytes);
    }
    return total_bytes;
}

void opm_compress(opm_convert_context_t* ctx) {
    ctx->opmpacked.resize(ctx->opmrecords.size());
    printf("compressing"); fflush(stdout);

    // messy backref bruteforce stuff
    struct backref_bruteforce_t {
        int min, max;
        int best; uint32_t bestsize;
    } backref_len;
    switch (ctx->flags.compress_level) {
    case 2: backref_len.min = 2; backref_len.max = 16; break;      // bruteforced
    case 1:
    default: backref_len.min = backref_len.max = 4; break;         // fixed 
    };

    int ch = 0;
    for (auto& src_ch : ctx->opmrecords) {
        if (ctx->flags.verbosity >= 2) {
            printf("\n");
        }
        backref_bruteforce_t cur_backref_len;
        cur_backref_len.min = backref_len.min;
        cur_backref_len.max = backref_len.max;
        cur_backref_len.best = backref_len.min;
        cur_backref_len.bestsize = -1;
        int sz;
        for (sz = cur_backref_len.min; sz <= cur_backref_len.max; sz++) {
            auto bytes = opm_compress_channel(ctx, ch, sz);
            if (cur_backref_len.bestsize >= bytes) {
                cur_backref_len.bestsize = bytes;
                cur_backref_len.best = sz;
            }
        }
        // recompress if current != best
        if (sz-1 != cur_backref_len.best) opm_compress_channel(ctx, ch, cur_backref_len.best);
        // calculate byte offsets for each frame
        std::vector<uint32_t> ch_bytepos;
        uint32_t bytepos_cur = 0;
        for (int f = 0; f < ctx->opmpacked[ch].size(); f++) {
            ch_bytepos.push_back(bytepos_cur);
            bytepos_cur += ctx->opmpacked[ch][f].rawdata.size();
        }
        // resolve back references
        for (int f = 0; f < ctx->opmpacked[ch].size(); f++) {
            // fixup back reference (if any)
            if (ctx->opmpacked[ch][f].flags & OPM_CHAN_BACKREF) {
                uint32_t backref_dist = ch_bytepos[f] - ch_bytepos[f - ctx->opmpacked[ch][f].distance_frames];
                ctx->opmpacked[ch][f].rawdata[0] = OPM_STREAM_BACKREF | (backref_dist >> 8);
                ctx->opmpacked[ch][f].rawdata[1] = (backref_dist & 0xFF);
            }
        }
        ch++;
#ifndef ULTRA_DEBUG
        printf("."); fflush(stdout);
#endif
    }
    printf("done\n");
}
