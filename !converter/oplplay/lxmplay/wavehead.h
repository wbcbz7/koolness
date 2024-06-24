#pragma once

#include <stdint.h>

#pragma pack(push, 1)

// RIFF header
struct RIFF_Header {
    char        id[4];      // "RIFF"
    uint32_t    size;
    char        fourcc[4];  // "WAVE"
};

struct chunk_Header {
    char        id[4];
    uint32_t    size;
};

// wave format header
struct fmt_Header {
    char        id[4];              // "fmt "
    uint32_t    size;               // size of chunk!

    uint16_t    wFormatTag;         // Format code
    uint16_t    nChannels;          // Number of interleaved channels
    uint32_t    nSamplesPerSec;     // Sampling rate (blocks per second)
    uint32_t    nAvgBytesPerSec;    // Data rate
    uint16_t    nBlockAlign;        // Data block size (bytes)
    uint16_t    wBitsPerSample;     // Bits per sample

    // enough :)
};

#pragma pack(pop)
