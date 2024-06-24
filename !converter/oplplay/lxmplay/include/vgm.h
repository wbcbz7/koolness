#pragma once

#include <stdint.h>

struct VGMHeader {
    char            id[4];              // "Vgm/x20"
    uint32_t        eofOffset;          // filesize - 4
    uint32_t        version;            // in BCD, latest is 0x170 -> 1.70

    // any "#clock" fields - 0 means chip is unused
    uint32_t        SN76489_Clock;
    uint32_t        YM2413_Clock;

    uint32_t        GD3_Offset;         // GD3 tags offset (ignored)

    uint32_t        samplesNum;         // Total of all wait values in the file. (umm?)
    uint32_t        loopOffset;         // relative from this field offset
    uint32_t        loopLength;         // 0 if no loop

    uint32_t        frameRate;          // NOT the chip sampling rate (it's 44100hz) but rather (in most cases) a player routine calling rate (usually 60hz for NTSC and 50hz for PAL systems), DON'T RELY ON!

    uint32_t        SN76489_Feedback:16;    // The white noise feedback pattern for the SN76489 PSG
    uint32_t        SN76489_ShiftWidth:8;   //  The noise feedback shift register width, in bits

    uint32_t        SN76489_Flags:8;        // Misc flags for the SN76489. Most of them don't make audible changes and can be ignored, if the SN76489 emulator lacks the features.
                                            // bit 0 	    frequency 0 is 0x400
                                            // bit 1        output negate flag
                                            // bit 2        stereo on / off(on when bit clear)
                                            // bit 3        / 8 Clock Divider on / off(on when bit clear)
                                            // bit 4 - 7    reserved(must be zero)
    uint32_t        YM2612_Clock;
    uint32_t        YM2151_Clock;

    uint32_t        dataOffset;         // relative from this field offset - use it for data fetching!

    uint32_t        unused[6];
    uint32_t        YM3812_Clock;
    uint32_t        unused_2[2];
    uint32_t        YMF262_Clock;
};


// stream opcodes descriptions

/* 
    general opcode groups (for faster and easier parsing):

        0x30..0x3F              - one operand
        0x40..0x4F, 0x50..0x5F,
        0xA0..0xAF, 0xB0..0xBF  - two operands (except for 0x4F/0x50, which are one operand)

        0xC0..0xCF, 0xD0..0xDF  - three operands
        0xE0..0xEF, 0xF0..0xFF  - four operands
*/

enum class VGM_Stream_Opcode : uint8_t {

    PSG_STEREO_WRITE    = 0x4F, // 0x4F dd    : Game Gear PSG stereo, write dd to port 0x06
    PSG_WRITE           = 0x50, // 0x50 dd    : PSG(SN76489 / SN76496) write value dd
    YM2413_WRITE        = 0x51, // 0x51 aa dd : YM2413, write value dd to register aa
    YM2612_PORT0_WRITE  = 0x52, // 0x52 aa dd : YM2612 port 0, write value dd to register aa
    YM2612_PORT1_WRITE  = 0x53, // 0x53 aa dd : YM2612 port 1, write value dd to register aa
    YM2151_WRITE        = 0x54, // 0x54 aa dd : YM2151, write value dd to register aa
    YM2203_WRITE        = 0x55, // 0x55 aa dd : YM2203, write value dd to register aa
    YM2608_PORT0_WRITE  = 0x56, // 0x56 aa dd : YM2608 port 0, write value dd to register aa
    YM2608_PORT1_WRITE  = 0x57, // 0x57 aa dd : YM2608 port 1, write value dd to register aa
    YM2610_PORT0_WRITE  = 0x58, // 0x58 aa dd : YM2610 port 0, write value dd to register aa
    YM2610_PORT1_WRITE  = 0x59, // 0x59 aa dd : YM2610 port 1, write value dd to register aa
    YM3812_WRITE        = 0x5a, // 0x5A aa dd : YM3812, write value dd to register aa
    YM3526_WRITE        = 0x5b, // 0x5B aa dd : YM3526, write value dd to register aa
    Y8950_WRITE         = 0x5c, // 0x5C aa dd : Y8950, write value dd to register aa
    YMZ280B_WRITE       = 0x5d, // 0x5D aa dd : YMZ280B, write value dd to register aa
    YMF262_PORT0_WRITE  = 0x5e, // 0x5E aa dd : YMF262 port 0, write value dd to register aa
    YMF262_PORT1_WRITE  = 0x5f, // 0x5F aa dd : YMF262 port 1, write value dd to register aa
    
    DELAY_LONG          = 0x61, // 0x61 nn nn : Wait n samples, n can range from 0 to 65535 (approx 1.49 seconds). Longer pauses than this are represented by multiple wait commands.
    DELAY_60HZ          = 0x62, // 0x62 : wait 735 samples(60th of a second), a shortcut for 0x61 0xdf 0x02
    DELAY_50HZ          = 0x63, // 0x63 : wait 882 samples(50th of a second), a shortcut for 0x61 0x72 0x03
    SET_DELAY_LENGTH    = 0x64, // 0x64 : cc nn nn : override length of 0x62 / 0x63:  cc - command(0x62 / 0x63),  nn - delay in samples [Note:Not yet implemented.Am I really sure about this ? ]
    END_OF_DATA         = 0x66, // 0x66 : end of sound data
    DATA_BLOCK          = 0x67, // 0x67 ... : data block : see below
    PCM_RAW_WRITE       = 0x68, // 0x68 ... : PCM RAM write : see below
    
    AY_WRITE            = 0xa0, // 0xA0 aa dd : AY8910, write value dd to register aa
    RF5C68_WRITE        = 0xb0, // 0xB0 aa dd : RF5C68, write value dd to register aa
    RF5C164_WRITE       = 0xb1, // 0xB1 aa dd : RF5C164, write value dd to register aa
    PWM_WRITE           = 0xb2, // 0xB2 ad dd : PWM, write value ddd to register a(d is MSB, dd is LSB)
    DMG_WRITE           = 0xb3, // 0xB3 aa dd : GameBoy DMG, write value dd to register aa
    APU_WRITE           = 0xb4, // 0xB4 aa dd : NES APU, write value dd to register aa
    MULTIPCM_WRITE      = 0xb5, // 0xB5 aa dd : MultiPCM, write value dd to register aa
    UPD7759_WRITE       = 0xb6, // 0xB6 aa dd : uPD7759, write value dd to register aa
    OKIM6258_WRITE      = 0xb7, // 0xB7 aa dd : OKIM6258, write value dd to register aa
    OKIM6295_WRITE      = 0xb8, // 0xB8 aa dd : OKIM6295, write value dd to register aa
    HUC6280_WRITE       = 0xb9, // 0xB9 aa dd : HuC6280, write value dd to register aa
    K053260_WRITE       = 0xba, // 0xBA aa dd : K053260, write value dd to register aa
    POKEY_WRITE         = 0xbb, // 0xBB aa dd : Pokey, write value dd to register aa
    SEGAPCM_WRITE       = 0xc0, // 0xC0 aaaa dd : Sega PCM, write value dd to memory offset aaaa
    RF5C68_MEM_WRITE    = 0xc1, // 0xC1 aaaa dd : RF5C68, write value dd to memory offset aaaa
    RF5C164_MEM_WRITE   = 0xc2, // 0xC2 aaaa dd : RF5C164, write value dd to memory offset aaaa
    MULTIPCM_SET_BANK   = 0xc3, // 0xC3 cc aaaa : MultiPCM, write set bank offset aaaa to channel cc
    QSOUND3_WRITE       = 0xc4, // 0xC4 mmll rr : QSound, write value mmll to register rr (mm - data MSB, ll - data LSB)
    YMF278B_WRITE       = 0xd0, // 0xD0 pp aa dd : YMF278B port pp, write value dd to register aa
    YMF271_WRITE        = 0xd1, // 0xD1 pp aa dd : YMF271 port pp, write value dd to register aa
    SCC1_WRITE          = 0xd2, // 0xD2 pp aa dd : SCC1 port pp, write value dd to register aa
    K054539_WRITE       = 0xd3, // 0xD3 pp aa dd : K054539 write value dd to register ppaa
    C140_WRITE          = 0xd4, // 0xD4 pp aa dd : C140 write value dd to register ppaa
    SEEK_TO_PCM_DATA    = 0xe0, // 0xE0 dddddddd : seek to offset dddddddd(Intel byte order) in PCM data bank

    // ranged opcodes
    DELAY_SHORT         = 0x70, // 0x7n : wait n + 1 samples, n can range from 0 to 15.
    YM2612_PCM_OUT      = 0x80, // 0x8n : YM2612 port 0 address 2A write from the data bank, then wait n samples; n can range from 0 to 15. Note that the wait is n, NOT n + 1. (Note: Written to first chip instance only.)
    DAC_STREAM_CONTROL  = 0x90, // 0x90 - 0x95  : DAC Stream Control Write : see below

};
