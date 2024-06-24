#pragma once
#include <stdint.h>

#pragma pack(push, 1)

struct opm_header_stream_desc_t {
    //uint16_t    ptr;        // offset to data stream in paragraphs (bytes*16)
    uint32_t    size;       // stream data size in bytes
};


struct opm_header_t {
    char    magic[4];       // "OPM\x1A"
    union {
        struct {
            uint8_t minor;
            uint8_t major;
        };
        uint16_t v;
    } version;
    uint16_t    flags;              // reserved
    uint16_t    frame_rate;         // [hz] = 0x1234dd/frame_rate
    uint8_t     callstack_depth;    // reserved, 0 at this moment
    uint8_t     reserved;

    // opm_header_stream_desc_t stream[9 + 1];  // first is control stream
};

// OPM v0 stream data:
enum {
    OPM_STREAM_END_FRAME        = 0xFF,     // end of frame, next channel
    OPM_STREAM_END              = 0xFE,     // end of stream, stop here or loop to OPM_STREAM_LOOP stream point
    OPM_STREAM_NOP              = 0xFD,
    OPM_STREAM_NEW_ORDER        = 0xFC,     // nop, marks new order
    OPM_STREAM_SET_FRAME_RATE   = 0xFB,     // word rate (as in opm_header_t::frame_rate)
    OPM_STREAM_LOOP             = 0xFA,     // set loop point here

    OPM_KEY_TRIGGER             = 0xF0,     // set key on/off + optionally end of frame

    // delay commands
    OPM_STREAM_DELAY_INT32      = 0xF9,     // dword delay
    OPM_STREAM_DELAY_INT16      = 0xF8,     // word  delay
    OPM_STREAM_DELAY_INT12      = 0xD0,     // D0..DF - 0..4095 frames delay (hibyte in low 4 bits of command)
    OPM_STERAM_DELAY_SHORT      = 0xC0,     // C0..CF - 1..16 frames delay

    // back reference 
    OPM_STREAM_BACKREF          = 0xE0,     // E0..EF - word backrefpos (12 bit), byte frames

    // setter commands
    OPM_SET_OPERATOR            = 0x00,     // 00..7F - set operator parameters
    OPM_SET_FREQ_FB_VOL         = 0x80,     // 80..BF - set frequency, feedback and total level

    // control register set
    OPM_CTRL_KEY_PERC           = 0x00,     // 00..7F - set key on/off for percussion, end of frame
    OPM_CTRL_REG_SET            = 0x80,     // 80..BF - set control registers

    // flags
    OPM_KEY_OFF                 = (0 << 0),
    OPM_KEY_ON                  = (1 << 0),
    OPM_KEY_END_OF_FRAME        = (1 << 1),

    OPM_SET_VOLUME_END_OF_FRAME = (1 << 7),

    OPM_CMD00_SET_MULT          = (1 << 0),
    OPM_CMD00_SET_TL            = (1 << 1),
    OPM_CMD00_SET_AD            = (1 << 2),
    OPM_CMD00_SET_SR            = (1 << 3),
    OPM_CMD00_SET_WAVEFORM      = (1 << 4),
    OPM_CMD00_SELECT_OPERATOR   = (1 << 5),
    OPM_CMD00_END_OF_FRAME      = (1 << 6),

    OPM_CMD80_SET_TL0           = (1 << 0),
    OPM_CMD80_SET_TL1           = (1 << 1),
    OPM_CMD80_SET_FEEDBACK      = (1 << 2),
    OPM_CMD80_SET_FREQ          = (1 << 3),
    OPM_CMD80_SET_KEYBLOCK      = (1 << 4),
    OPM_CMD80_END_OF_FRAME      = (1 << 5),

    OPM_CTRL_SET_REG01      = (1 << 0),
    OPM_CTRL_SET_REG08      = (1 << 1),
    OPM_CTRL_SET_REG105     = (1 << 2),
    OPM_CTRL_SET_REG104      = (1 << 3),
    OPM_CTRL_SET_REGBD      = (1 << 4),
};

#pragma pack(pop)
