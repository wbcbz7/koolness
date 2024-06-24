#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <math.h>

#include "cmdline.h"
#include "opmfile.h"
#include "opmctx.h"
#include "vgm.h"
#include "compressor.h"

opm_convert_context_t convert_ctx;
opm_convert_context_t* ctx = &convert_ctx;

uint32_t gdp_dump_delays(std::vector<uint8_t>& file, uint32_t start, uint32_t end, std::map<int, int> &delaystat, int min_delay) {
    uint32_t framerate = 0, delay_acc = 0, delay_rate;
    auto it = file.data() + start;
    while (it < file.data() + end) {
        auto it_u32 = (uint32_t*)it;
        // skip loop and chip markers
        if (*it_u32 == 0xFFFFFFFD) 
            it += 4;         // loop point?
        else if (*it_u32 <= 0x80000000) 
            it += 12;   // chip-reg-data tuple
        else if (*it_u32 == 0xFFFFFFFE) {
            delay_rate = *(it_u32+1);
            delaystat[delay_rate]++;        // no accumulators (known furnace dump)
            it += 8;
        }
        else {
            printf("unknown token!\n");
            exit(0);
        }
    }

    return framerate;
}

// 1st step: estimate most suitable frame rate
int opm_estimate_frame_rate(opm_convert_context_t *ctx) {
    // pick possible match
    ctx->delay = 44100.0 / 60.0;

    // gather statistics
    std::map<int, int> delaystat;
    gdp_dump_delays(ctx->vgm.vgmfile, ctx->vgm.start, ctx->vgm.end, delaystat, 44100 / ctx->estimate.max_delay_freq);

    // calculate total delay count
    uint32_t total_delays = 0; for (const auto& a : delaystat) total_delays += a.second;
    uint32_t delay_threshold = total_delays * ctx->estimate.min_delay_threshold;
    uint32_t trim_threshold = total_delays * ctx->estimate.trim_threshold;

    std::vector<std::pair<int, int>> delaystat_sorted(delaystat.begin(), delaystat.end());
    std::sort(delaystat_sorted.begin(), delaystat_sorted.end(), [](auto& a, auto& b) { return a.second > b.second; });

    // find minimum element within 1% of distribution
    int min_delay = INT_MAX; for (const auto& a : delaystat) if (a.second > delay_threshold) min_delay = std::min(min_delay, a.first);

    // trim all entries below trim threshold
    auto trim_boundary = std::find_if(delaystat_sorted.begin(), delaystat_sorted.end(), [&trim_threshold](auto& a) {return a.second <= trim_threshold; });
    delaystat_sorted.resize(trim_boundary - delaystat_sorted.begin());

    // rescale all delays
    std::vector<std::pair<double, int>> delaystat_rescaled;
    for (const auto& a : delaystat_sorted) {
        std::pair<double, int> val = a;
        double ratio = double(val.first) / min_delay;
        if (ratio > ctx->estimate.rescale_min_ratio) {
            val.first /= round(ratio);
        };
        delaystat_rescaled.push_back(val);
    }

    // calculate cumulative delay
    std::pair<double, int> total_delay = { 0, 0 };
    for (const auto& a : delaystat_rescaled) {
        total_delay.first += a.first * a.second;
        total_delay.second += a.second;
    }
    total_delay.first /= total_delay.second;

    // clamp to largest possible delay
    if (total_delay.first > ctx->estimate.max_delay_base) {
        total_delay.first /= ceil(total_delay.first / ctx->estimate.max_delay_base);
    }

    // and save it in the context
    ctx->delay = total_delay.first;

    return 0;
}

// ----------------------------------------------
int slotParamsToSlot[] = {
        0,  0,  0,  1,  1,  1, -1, -1,  0,  0,  0,  1,  1,  1, -1, -1,
        0,  0,  0,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

int slotParamsToChan[32] = {
    0,  1,  2,  0,  1,  2, -1, -1,  3,  4,  5,  3,  4,  5, -1, -1,
    6,  7,  8,  6,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
int slotParamsToIndex[32] = {
    0,  0,  0,  3,  3,  3, -1, -1,  0,  0,  0,  3,  3,  3, -1, -1,
    0,  0,  0,  3,  3,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int chanParamsToChan[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, -1, -1, -1, -1, -1, -1, -1
};

// ------------------------
// 2nd step - extract register writes from VGM, requantize to new frames
int opm_requantize(opm_convert_context_t* ctx) {
    ctx->oplchan.resize(1 + 18);     // TODO: FIXME for OPL3 support!!

    // per-channel structures
    std::vector<opm_channel_rawdata_t> oplchans(1 + 18);     // TODO
    uint32_t currentFrame = 0; uint32_t delay_rate, delay_acc = 0;

    auto it = ctx->vgm.vgmfile.data() + ctx->vgm.start; bool loopFrame = false;
    while (it < ctx->vgm.vgmfile.data() + ctx->vgm.end) {
        auto it_u32 = (uint32_t*)it;

        // flush delays
        delay_rate = 0;
        switch (*it_u32) {
            case 0x63:      // esfm write
                // split to channels
                {
                    int channel;
                    uint32_t reg  = *(it_u32 + 1);
                    uint32_t data = *(it_u32 + 2);
                    // key on range?
                    if ((reg >= 0x240) && (reg <= 0x253)) {
                        channel = reg - 0x240;
                        if (channel >= 16) channel = ((channel - 16) >> 1) + 16; // force all 4 ops for key change for ch16/17
                        oplchans[channel + 1].frame = currentFrame;
                        oplchans[channel + 1].data.push_back(OPM_KEY_TRIGGER | (data & 1));
                    }
                    else if (reg < 0x240) {
                        // channel regs
                        channel = reg >> 5;                         // 8*4 - 32 regs per channel
                        oplchans[channel + 1].frame = currentFrame;
                        oplchans[channel + 1].data.push_back(reg & 31);
                        oplchans[channel + 1].data.push_back(data);
                    }
                }
                it += 12;
                break;
            case 0xFFFFFFFE: // delay
                delay_rate = *(it_u32 + 1);
                it += 8;
                break;
            case 0xFFFFFFFD: // loop point
                // emit loop token
                loopFrame = true;
                for (int i = 0; i < 18 + 1; i++) {
                    oplchans[i].frame = currentFrame;
                }
                it += 4;
                break;
            default:
                it += 12;       // possibly other systems
                break;
        }

        if (delay_rate != 0) {
            // process possible delays
            int ratio = round(((double)delay_rate + delay_acc) / ctx->delay);
            double norm_delay = abs(((delay_rate + delay_acc) / (ratio > 0 ? ratio : 1)) - ctx->delay) / ctx->delay;
            if (norm_delay < 0.3) {
                // flush delay!
                delay_acc = 0;
                for (int ch = 0; ch < oplchans.size(); ch++) if (oplchans[ch].frame != -1) {
                    oplchans[ch].loop = loopFrame;
                    ctx->oplchan[ch].push_back(oplchans[ch]);
                    oplchans[ch].data.clear();
                    oplchans[ch].frame = -1;
                }
                loopFrame = false;
                // and increment current frame
                currentFrame += ratio;
            }
            else {
                delay_acc += delay_rate;
            }
            delay_rate = 0;
        }
    }

    // save total count of frames
    if (ctx->oplchan[0].back().frame == currentFrame) {
        ctx->oplchan[0].back().data.push_back(OPM_STREAM_END);
    }
    else {
        opm_channel_rawdata_t endmarker;
        endmarker.frame = currentFrame;
        endmarker.loop = false;
        endmarker.data.push_back(OPM_STREAM_END);
        ctx->oplchan[0].push_back(endmarker);
    }
    ctx->total_frames = currentFrame;

    return 0;
}

#if 1
// --------------------------
// 3rd step - output very very simple and uncompressed format (just for testing :)
// FF    - end of frame
// FE    - end of stream
// FD nn - wait N+2 frames
// rr dd - write dd to register rr (adjusted for current channel of course)
int opm_dump_simple(opm_convert_context_t* ctx) {
    ctx->oplchan_out.resize(1 + 18);     // TODO: FIXME for OPL3 support!!
    for (int ch = 0; ch < (1 + 18); ch++) {
        // dequeue the streams
        int delay_old = -1;
        for (int f = 0; f < ctx->oplchan[ch].size(); f++) {
            // copy raw register data
            ctx->oplchan_out[ch].insert(ctx->oplchan_out[ch].end(), ctx->oplchan[ch][f].data.begin(), ctx->oplchan[ch][f].data.end());
            // post either delay or end of stream
            if ((f == ctx->oplchan[ch].size() - 1)) {
                // post end of steram
                ctx->oplchan_out[ch].push_back(0xFE);
            } else {
                int delay = ctx->oplchan[ch][f + 1].frame - ctx->oplchan[ch][f].frame;
                //if (delay_old != delay) {
                    //delay_old =  delay;
                    do {
                        ctx->oplchan_out[ch].push_back(OPM_STREAM_DELAY_INT16);
                        ctx->oplchan_out[ch].push_back(delay & 0xFF);
                        ctx->oplchan_out[ch].push_back(delay >> 8);
                        delay -= 65536;
                    } while (delay > 0);
                //}
                ctx->oplchan_out[ch].push_back(0xFF);
            }
        }
    }

    return 0;
}

// --------------------------
// 4th step - write OPM file
int opm_write_file(opm_convert_context_t* ctx) {
    struct pad_info_t {
        uint32_t pos, pad;
    };
    // fill write info
    auto round_to_para = [](uint32_t a) -> pad_info_t { return { (a + 15) & ~15 , ((a + 15) & ~15) - a }; };

    memcpy(ctx->opmfile.header.magic, "OPM\x1A", 4);
    ctx->opmfile.header.version.v = 0x0001;
    ctx->opmfile.header.flags = 0;
    ctx->opmfile.header.callstack_depth = 0;
    ctx->opmfile.header.frame_rate = ((double)0x1234DD / (double)(44100 / ctx->delay));
    ctx->opmfile.header.reserved = 0;

    struct opm_write_file_info_t {
        uint32_t pos;
        uint32_t padding_pre;
        uint32_t size;
    };
    std::vector<opm_write_file_info_t> writeinfo(18 + 1);
    ctx->opmfile.streamdesc.resize(18 + 1);

#if 0
    // calculate offsets
    auto fsize = round_to_para(sizeof(ctx->opmfile.header) + (9 + 1) * sizeof(opm_header_stream_desc_t));
    for (int i = 0; i < 9 + 1; i++) {
        writeinfo[i].pos = fsize.pos;
        writeinfo[i].padding_pre = fsize.pad;
        writeinfo[i].size = ctx->oplchan_out[i].size();

        ctx->opmfile.streamdesc[i].ptr = (fsize.pos >> 4);
        ctx->opmfile.streamdesc[i].size = ctx->oplchan_out[i].size();

        fsize = round_to_para(fsize.pos + ctx->oplchan_out[i].size());
    }

    // dump to OPM file
    FILE* f = fopen(ctx->opmfile.filename.c_str(), "wb");
    if (!f) return 1;

    // write header
    fwrite(&ctx->opmfile.header, sizeof(opm_header_t), 1, f);
    fwrite(ctx->opmfile.streamdesc.data(), sizeof(decltype(ctx->opmfile.streamdesc)::value_type), ctx->opmfile.streamdesc.size(), f);
    // write (aligned) channel streams
    for (int i = 0; i < 1 + 9; i++) {
        if (writeinfo[i].padding_pre > 0) for (int c = 0; c < writeinfo[i].padding_pre; c++) fputc(0, f);   // dirty!!!
        fwrite(ctx->oplchan_out[i].data(), sizeof(uint8_t), ctx->oplchan_out[i].size(), f);
    }
    fclose(f);
#else
    for (int i = 0; i < 18 + 1; i++) {
        ctx->opmfile.streamdesc[i].size = ctx->oplchan_out[i].size();
    }
    // dump to OPM file
    FILE* f = fopen(ctx->opmfile.filename.c_str(), "wb");
    if (!f) return 1;

    // write header
    fwrite(&ctx->opmfile.header, sizeof(opm_header_t), 1, f);
    fwrite(ctx->opmfile.streamdesc.data(), sizeof(decltype(ctx->opmfile.streamdesc)::value_type), ctx->opmfile.streamdesc.size(), f);
    // write channel streams
    for (int i = 0; i < 1 + 18; i++) {
        fwrite(ctx->oplchan_out[i].data(), sizeof(uint8_t), ctx->oplchan_out[i].size(), f);
    }
    fclose(f);
#endif
}

#endif

int opm_group_control_stream(opm_convert_context_t* ctx) {
    // default (startup) frame record
    opm_frame_record defrec;
    memset(&defrec, -1, sizeof(defrec));

    for (int f = 0; f < ctx->oplchan[0].size(); f++) {
        opm_channel_record_t chanrec;
        chanrec.frame = ctx->oplchan[0][f].frame;
        chanrec.frame_dist = f >= ctx->oplchan[0].size() - 1 ? 0 : ctx->oplchan[0][f + 1].frame - ctx->oplchan[0][f].frame;
        chanrec.flags = 0;
        if (ctx->oplchan[0][f].loop) chanrec.flags |= OPM_CHAN_LOOP_POINT;
        defrec.flags = 0;

        // nothing to parse! really
        ctx->opmrecords[0].push_back(chanrec);
    }

    return 0;
}

int opm_group_channel_stream(opm_convert_context_t* ctx, int ch) {
    // default (startup) frame record
    opm_frame_record defrec;
    memset(&defrec, -1, sizeof(defrec));

    for (int f = 0; f < ctx->oplchan[ch].size(); f++) {
        opm_channel_record_t chanrec;
        chanrec.frame      = ctx->oplchan[ch][f].frame;
        chanrec.frame_dist = f >= ctx->oplchan[ch].size()-1 ? 0 : ctx->oplchan[ch][f+1].frame - ctx->oplchan[ch][f].frame;
        chanrec.flags = 0;
        if (ctx->oplchan[ch][f].loop) chanrec.flags |= OPM_CHAN_LOOP_POINT;
        defrec.flags = 0;
        defrec.esfm_flags = 0;

#if 1
        chanrec.ultrarawdata.insert(chanrec.ultrarawdata.end(), ctx->oplchan[ch][f].data.begin(), ctx->oplchan[ch][f].data.end());
#else
        // parse register writes
        bool isFrame = true; auto it = ctx->oplchan[ch][f].data.begin();
        while (isFrame && (it < ctx->oplchan[ch][f].data.end())) {
            // get reg:data pair
            int reg = *it++; if (reg == OPM_STREAM_END) break;
            if (reg < 0x20) {
                defrec.esfm_flags |= (1 << reg);
                defrec.esfm_regs[reg] = *it++;
            }
            else if (reg & OPM_KEY_TRIGGER) {
                defrec.flags |= OPM_KEY;
                defrec.key = reg & 1;
                // SERIALIZE!
                chanrec.records.push_back(defrec); defrec.flags = 0; defrec.esfm_flags = 0;
            }
        }
#endif
#if 0
        if ((defrec.flags != 0) || (defrec.esfm_flags != 0) || (true)) {
            chanrec.records.push_back(defrec); defrec.flags = 0; defrec.esfm_flags = 0;
        }
#endif
        ctx->opmrecords[ch].push_back(chanrec);
    }

    return 0;
}

// --------------------------
// 3rd step - group events by registers written
// key changes are acting as "fence" (the entire stream is flushed in this case)
int opm_group_registers(opm_convert_context_t* ctx) {
    ctx->opmrecords.resize(18 + 1);

    opm_group_control_stream(ctx);
    for (int ch = 0; ch < 18; ch++) {
        opm_group_channel_stream(ctx, ch+1);
    }

    return 0;
}

// set delay 
void opm_set_delay(std::vector<uint8_t>& vec, uint64_t delay) {
    if (delay == 0) return;
    else if ((delay > 0) && (delay < 17)) {
        vec.push_back(OPM_STERAM_DELAY_SHORT + (delay - 1));
    }
    else if (delay < 4096) {
        vec.push_back(OPM_STREAM_DELAY_INT12 + (delay >> 8));
        vec.push_back(delay & 0xFF);
    }
    else if (delay < 65536) {
        vec.push_back(OPM_STREAM_DELAY_INT16);
        vec.push_back((delay >> 0) & 0xFF);
        vec.push_back((delay >> 8) & 0xFF);
    }
    else {
        vec.push_back(OPM_STREAM_DELAY_INT32);
        vec.push_back((delay >> 0) & 0xFF);
        vec.push_back((delay >> 8) & 0xFF);
        vec.push_back((delay >> 16) & 0xFF);
        vec.push_back((delay >> 24) & 0xFF);
    }
};

int opm_serialize_control_stream(opm_convert_context_t* ctx) {
    int old_delay = -1;
    uint32_t byte_stamp = 0;
    for (auto& s : ctx->opmrecords[0]) {
        s.rawdata.clear();
        
        // set loop point
        if (s.flags & OPM_CHAN_LOOP_POINT) s.rawdata.push_back(OPM_STREAM_LOOP);

        // set delay
        if (s.frame_dist != old_delay) {
            old_delay = s.frame_dist;
            opm_set_delay(s.rawdata, s.frame_dist);
        }

        // process events (nothing to process!)
        s.rawdata.push_back(s.frame_dist == 0 ? OPM_STREAM_END : OPM_STREAM_END_FRAME);
        //s.rawdata.push_back(OPM_STREAM_END_FRAME);

        // save byte stamp
        s.byte_stamp = byte_stamp;
        byte_stamp += s.rawdata.size();
    }
    return 0;
}

int opm_serialize_channel_stream(opm_convert_context_t* ctx, int ch) {
    int old_delay = -1;
    uint32_t byte_stamp = 0;
    for (auto& s : ctx->opmrecords[ch]) {
        s.rawdata.clear();

        // set loop point
        if (s.flags & OPM_CHAN_LOOP_POINT) s.rawdata.push_back(OPM_STREAM_LOOP);

        // set delay
        if (s.frame_dist != old_delay) {
            old_delay = s.frame_dist;
            opm_set_delay(s.rawdata, s.frame_dist);
        }

#if 0
        if (s.records.size() > 0) for (int i = 0; i < s.records.size(); i++) {
#if 1
            auto& e = s.records[i];
            //for (const auto& rd : e.rawdata) s.rawdata.push_back(rd);
#else
            auto& e = s.records[i];
            for (int i = 0; i < 32; i++) {
                if (e.esfm_flags & (1 << i)) {
                    s.rawdata.push_back(i);
                    s.rawdata.push_back(e.esfm_regs[i]);
                }
            }
            if (e.flags & OPM_KEY) {
                s.rawdata.push_back(
                    OPM_KEY_TRIGGER | e.key
                );
            }
#endif
        }
#else
        s.rawdata.insert(s.rawdata.end(), s.ultrarawdata.begin(), s.ultrarawdata.end());
#endif
        s.rawdata.push_back(s.frame_dist == 0 ? OPM_STREAM_END : OPM_STREAM_END_FRAME);

        // save byte stamp
        s.byte_stamp = byte_stamp;
        byte_stamp += s.rawdata.size();
    }
    return 0;
}

// --------------------------
// 4th step - generate byte stream
int opm_serialize_stream(opm_convert_context_t* ctx) {
    opm_serialize_control_stream(ctx);
    for (int ch = 0; ch < 18; ch++) {
        opm_serialize_channel_stream(ctx, ch + 1);
    }
    return 0;
}

// --------------------------
// 5th step - not compress yet :) concatenate byte streams of all frames
int opm_concat_streams(opm_convert_context_t* ctx) {
    ctx->oplchan_out.resize(1 + 18);     // TODO: FIXME for OPL3 support!!
    for (int ch = 0; ch < (1 + 18); ch++) {
        for (auto& f : (ctx->flags.compress_level == 0) ? ctx->opmrecords[ch] : ctx->opmpacked[ch]) {
            // copy raw register data
            ctx->oplchan_out[ch].insert(ctx->oplchan_out[ch].end(), f.rawdata.begin(), f.rawdata.end());
        }
        ctx->oplchan_out[ch].push_back(OPM_STREAM_END);
    }

    return 0;
}

int opm_dump_backref(opm_convert_context_t* ctx, FILE* f, int ch, int pos, int frames, int current_frame) {
    do {
        auto& a = ctx->opmpacked[ch][pos];
        if (a.flags & OPM_CHAN_BACKREF) {
            current_frame = opm_dump_backref(ctx, f, ch, pos - a.distance_frames, a.frames_to_play, current_frame);
        }
        else {
            fprintf(f, "frame %07d, dist %d: data ", current_frame, a.frame_dist);
            for (auto& d : a.rawdata) fprintf(f, "%02X ", d);
            fprintf(f, "\n");
            current_frame += a.frame_dist;
        }
        pos++;
    } while ((--frames != 0) && (pos < ctx->opmpacked[ch].size()));
    return current_frame;
}

// ---------------------------
// dump events for each channel
int opm_dump_events(opm_convert_context_t* ctx) {
    FILE* f = fopen(ctx->logname.c_str(), "w");
    fprintf(f, "total %d frames\n", ctx->total_frames);
    for (int ch = 0; ch < (1 + 18); ch++) {
        fprintf(f, "---channel %d\n", ch-1);
        for (auto& a : ctx->opmrecords[ch]) {
            fprintf(f, "frame %07d, dist %d: data ", a.frame, a.frame_dist);
            for (auto& d : a.rawdata) fprintf(f, "%02X ", d);
            fprintf(f,"\n");
        }
    }
    fclose(f);

    f = fopen((ctx->logname+".packed.log").c_str(), "w");
    if (ctx->flags.compress_level > 0) {
        fprintf(f, "total %d frames\n", ctx->total_frames);
        for (int ch = 0; ch < (1 + 18); ch++) {
            fprintf(f, "---channel %d\n", ch - 1);
            if (ctx->opmpacked[ch].size() > 0) for (auto& a : ctx->opmpacked[ch]) {
                fprintf(f, "frame %07d, dist %d: data ", a.frame, a.frame_dist);
                for (auto& d : a.rawdata) fprintf(f, "%02X ", d);
                fprintf(f, "\n");
            }
        }
    }
    fclose(f);

    // dump decompressed statistics
    f = fopen((ctx->logname + ".unpacked.log").c_str(), "w");
    if (ctx->flags.compress_level > 0) {
        fprintf(f, "total %d frames\n", ctx->total_frames);
        for (int ch = 0; ch < (1 + 18); ch++) {
            fprintf(f, "---channel %d\n", ch - 1);
            opm_dump_backref(ctx, f, ch, 0, -1, 0);
        }
    }
    fclose(f);
    return 0;
}

// -------------------------

const cmdline_t cmdparams[] = {
    {'C',   CMD_FLAG_INT,      "COMPRESSION",       &ctx->flags.compress_level},
    {'D',   CMD_FLAG_INT,      "DEPTH",             &ctx->flags.max_stack_depth},
    {'V',   CMD_FLAG_INT,      "VERBOSITY",         &ctx->flags.verbosity},
};

// ----------------

int main(int argc, char* argv[]) {
    // clear compression context!
    ctx->flags.compress_level   = 1;
    ctx->flags.max_stack_depth  = 1;
    ctx->flags.verbosity        = 1;

    if (argc < 2) {
        printf("usage: vgm2opm input.vgm [-cx] [-dx] [-vx]\n");
        printf("-c[x]: enable compression, [x] - mode (default - 1):\n");
        printf("       0 - disabled, 1 - fixed min backref, 2 - bruteforce best backref\n");
        printf("-d[x]: maximum back reference stack depth (default - 2)\n");
        printf("-v[x]: verbosity level (default - 1)\n");
        return 1;
    }

    // read parameters
#if 1
    if (parse_cmdline(argc, argv, cmdparams, 3, 2) != 0) {
        printf("error: unable to parse command line!\n");
        return 1;
    }
#endif

    // file names
    std::string infile_str = argv[1];
    std::string outfile_str = infile_str.substr(0, infile_str.find(".gdp")) + ".opm";
    std::string csvfile_str = infile_str.substr(0, infile_str.find(".gdp")) + ".csv";
    std::string logfile_str = infile_str.substr(0, infile_str.find(".gdp")) + ".log";
    ctx->opmfile.filename = outfile_str;
    ctx->logname = logfile_str;

    std::ifstream infile(infile_str, std::ios::in | std::ios::binary);
    infile.unsetf(std::ios::skipws);

    // get filesize
    infile.seekg(0, std::ios::end);
    uint64_t fsize = infile.tellg();
    infile.seekg(0, std::ios::beg);

    // read whole file
    ctx->vgm.vgmfile.reserve(fsize);
    ctx->vgm.vgmfile.insert(ctx->vgm.vgmfile.begin(), std::istream_iterator<uint8_t>(infile), std::istream_iterator<uint8_t>());

    // get header
    ctx->vgm.header = reinterpret_cast<VGMHeader*>(ctx->vgm.vgmfile.data());

    // parse basic VGM structure
    ctx->vgm.end = fsize;
    ctx->vgm.start = 24;    // skip header

    // set estimation parameters
    ctx->estimate.max_delay_base = (44100.0 / ((double)0x1234DD / 65536));
    ctx->estimate.max_delay_freq = 400.0;
    ctx->estimate.trim_threshold = 0.005;
    ctx->estimate.min_delay_threshold = 0.01;
    ctx->estimate.rescale_min_ratio = 1.5;

    // and estimate it
    opm_estimate_frame_rate(ctx);
    printf("estimated frame rate = %.1lf samples [%.3lf Hz]\n", ctx->delay, 44100.0 / ctx->delay);

    // requantize and collect per-channel data
    opm_requantize(ctx);

#if 0
    // group registers
    opm_dump_simple(ctx);

    // dump OPM file
    opm_write_file(ctx);
#else
    // group registers
    opm_group_registers(ctx);

    // serialize to sequence of bytes
    opm_serialize_stream(ctx);

    // coplress the stream
    if (ctx->flags.compress_level > 0) opm_compress(ctx);

    // concatenate streams
    opm_concat_streams(ctx);

    // dump raw data
    opm_dump_events(ctx);

    // dump OPM file
    opm_write_file(ctx);
#endif

    // write statistics
    // TODO

    printf("done\n");

    return 0;
}
