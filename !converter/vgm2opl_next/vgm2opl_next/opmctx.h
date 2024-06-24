#pragma once
#include <stdint.h>
#include "opmfile.h"
#include "vgm.h"

#include <vector>
#include <string>
#include <map>

enum {
    LXM_CTRL_START = 0,
    LXM_CTRL_END,
    LXM_CTRL_SET_FRAME_RATE,
    LXM_CTRL_EVENT_MASK = (1 << 16) - 1,

    LXM_CTRL_LOOP = (1 << 16),
};

struct opm_control_track_t {
    uint32_t frame;
    int type, data;
};

/*
    oof
    в общем, для каждого фрейма понадобится хранить цепочку событий, разделяемых
    записью в key on (если таковая есть), поскольку для ретрига ноты надо сначала
    сборсить key on в 0, затем поставить в 1
*/

enum {
    OPM_KEY             = (1 << 13),
};

struct opm_frame_record {
    int flags;
    int key;

    // esfm stuff per channel
    uint32_t esfm_flags;
    uint8_t esfm_regs[32];
};

enum {
    OPM_CHAN_NEWPATTERN = (1 << 0),
    OPM_CHAN_LOOP_POINT = (1 << 1),

    OPM_CHAN_BACKREF    = (1 << 8),
};

struct opm_channel_record_t {
    // current frame index
    uint32_t frame;
    // distance between next and this frame
    uint32_t frame_dist;
    // flags
    int flags;
    // raw data
    std::vector<uint8_t> rawdata;
    // ultra raw data
    std::vector<uint8_t> ultrarawdata;
    // records
    std::vector<opm_frame_record> records;
    // compression data
    int distance_frames, distance_bytes, frames_to_play, frames_to_play_total;
    // byte stamp
    uint32_t byte_stamp;
};


struct opm_framerate_estimate_t {
    float max_delay_freq;           // maximum possible frequency, delays faster are concatenated together
    float max_delay_base;           // maximum base delay allowed
    float min_delay_threshold;      // percentile of delays used to find the minimum delay
    float trim_threshold;           // delays with occurence lower than threshld will be trimmed
    float rescale_min_ratio;        // used to rescale delays to one base
};

struct vgm_context_t {
    std::vector<uint8_t> vgmfile;
    // there is no actual header in gdp file
    VGMHeader* header;
    uint32_t loop_pos;
    uint32_t start, end;        // offsets
};

struct opm_channel_rawdata_t {
    // current frame index
    uint32_t frame;
    // loop flag
    bool     loop;
    // raw OPL data
    std::vector<uint8_t> data;
};

struct opm_convert_context_t {
    // VGM context
    vgm_context_t vgm;

    // conversion flags
    struct {
        int  compress_level;
        int  max_stack_depth;
        int  verbosity;
    } flags;

    // frame rate stuff
    double                   delay;
    opm_framerate_estimate_t estimate;

    // total frames
    uint32_t                 total_frames;

    // raw per-channel OPL data
    std::vector<std::vector<opm_channel_rawdata_t>> oplchan;
    std::vector<std::vector<uint8_t>> oplchan_out;

    // -----------------------------------

    // channel records
    std::vector<std::vector<opm_channel_record_t>> opmrecords;
    std::vector<opm_control_track_t> opmctrl;
    // channel bytes per blabla
    std::vector<int> opmrecords_bytes;

    // compressed streams
    std::vector<std::vector<opm_channel_record_t>> opmpacked;

    // final OPM stream
    std::vector<std::vector<size_t>>  opmstream_pos; // positions for each events, used for backref pos calc
    std::vector<std::vector<uint8_t>> opmstream;

    // -----------------------------------
    struct {
        std::string     filename;
        opm_header_t    header;
        std::vector<opm_header_stream_desc_t> streamdesc;
    } opmfile;

    std::string     logname;
};

