#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>

#include <stdint.h>
#include <portaudio.h>
#include <math.h>
#include <vector>
#include <fstream>

#include "wavehead.h"
#include "opmplay.h"
#include "esfm.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define NOMINMAX
#include <Windows.h>

const uint32_t SAMPLE_RATE = 49716;
enum {
    CHANNELS = 2,
    FRAMES_PER_BUFFER = 1024,
};

PaStreamParameters outputParameters;
PaStream* stream;

// console stuff
struct {
    HANDLE hStdout, hScreenBuffer;
    COORD bufcoord, bufsize;
    SMALL_RECT bufDestRect;
    CHAR_INFO* buffer;          // the main buffer to write to
} console;

struct opm_context_t {
    // full context
    opmplay_context_t opm;
    opmplay_io_t      io;

    // delay count relative to sample rate
    int32_t           delay_count;
};

opm_context_t opmctx;
esfm_chip esfm;


// ------------------

// draw plain string
void drawstring(const char* str, unsigned long x, unsigned long y, unsigned char attr) {
    CHAR_INFO* p = (CHAR_INFO*)console.buffer + (console.bufsize.X * y) + x;

    while (*str != '\0') {
        p->Char.AsciiChar = *str++;
        p->Attributes = attr;
        p++;
    }
}

// draw string with attributes
// '\0' - end, '\xFF\xaa' - set attribute byte 'aa'
void drawastring(const char* str, unsigned long x, unsigned long y) {
    CHAR_INFO* p = (CHAR_INFO*)console.buffer + (console.bufsize.X * y) + x;

    unsigned short attr = 0x07;

    while (*str != '\0') if (*str == '\xFF') {
        attr = (*++str); str++;
    }
    else {
        p->Char.AsciiChar = *str++;
        p->Attributes = attr;
        p++;
    }
}

// printf 
int tprintf(uint32_t x, uint32_t y, const char* format, ...) {
    char buffer[1024];      // large enough
    va_list arglist;

    va_start(arglist, format);
    int rtn = vsnprintf(buffer, sizeof(buffer), format, arglist);
    drawastring(buffer, x, y);
    va_end(arglist);

    return rtn;
};

// -------------------


int console_open() {
    // Get a handle to the STDOUT screen buffer to copy from and
    // create a new screen buffer to copy to.

    console.hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    console.hScreenBuffer = CreateConsoleScreenBuffer(
        GENERIC_READ |           // read/write access
        GENERIC_WRITE,
        FILE_SHARE_READ |
        FILE_SHARE_WRITE,        // shared
        NULL,                    // default security attributes
        CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE
        NULL);                   // reserved; must be NULL
    if (console.hStdout == INVALID_HANDLE_VALUE ||
        console.hScreenBuffer == INVALID_HANDLE_VALUE)
    {
        printf("CreateConsoleScreenBuffer failed - (%d)\n", GetLastError());
        return 1;
    }

    // resize
    console.bufsize.X = 80;
    console.bufsize.Y = 40;
    SetConsoleScreenBufferSize(console.hScreenBuffer, console.bufsize);

    // allocate console buffer
    console.buffer = new CHAR_INFO[console.bufsize.X * console.bufsize.Y];
    memset(console.buffer, 0, sizeof(CHAR_INFO) * console.bufsize.X * console.bufsize.Y);

    // Make the new screen buffer the active screen buffer.
    if (!SetConsoleActiveScreenBuffer(console.hScreenBuffer))
    {
        printf("SetConsoleActiveScreenBuffer failed - (%d)\n", GetLastError());
        return 1;
    }

    return 0;
}

void console_update() {
    console.bufDestRect.Top = 0;
    console.bufDestRect.Left = 0;
    console.bufDestRect.Bottom = console.bufsize.Y - 1;
    console.bufDestRect.Right = console.bufsize.X - 1;

    console.bufcoord.X = console.bufcoord.Y = 0;

    WriteConsoleOutput(
        console.hScreenBuffer,  // screen buffer to write to
        console.buffer,         // buffer to copy from
        console.bufsize,        // col-row size of chiBuffer
        console.bufcoord,       // top left src cell in chiBuffer
        &console.bufDestRect);  // dest. screen buffer rectangle
}

void console_done() {
    SetConsoleActiveScreenBuffer(console.hStdout);
}

// -------------------
// opl3 synth render
int synth_render(int16_t* buffer, uint32_t num_samples) {
    int samples_to_render = num_samples;

    while (samples_to_render > 0) {
        if (samples_to_render < opmctx.delay_count) {
            ESFM_generate_stream(&esfm, buffer, samples_to_render);
            opmctx.delay_count -= samples_to_render;
            break;
        }
        else {
            // calculate new delay
            ESFM_generate_stream(&esfm, buffer, opmctx.delay_count);
            buffer += CHANNELS * opmctx.delay_count;
            samples_to_render -= opmctx.delay_count;
            
            // parse VGM stream
            opmplay_tick(&opmctx.opm, &esfm);
            opmctx.delay_count = (SAMPLE_RATE / ((double)0x1234DD / opmctx.opm.header.frame_rate));
        }
    }

    return 0;
}


int pa_init() {
    PaError err;

    // init portaudio
    err = Pa_Initialize();
    if (err != paNoError) return 1;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        return 1;
    }
    outputParameters.channelCount = CHANNELS;
    outputParameters.sampleFormat = paInt16;
    outputParameters.suggestedLatency = 0.04;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &stream,
        NULL, /* no input */
        &outputParameters,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        0,     /* we won't output out of range samples so don't bother clipping them */
        NULL, /* no callback, use blocking API */
        NULL); /* no callback, so no callback userData */
    if (err != paNoError) return 1;

    err = Pa_StartStream(stream);
    if (err != paNoError) return 1;

    return 0;
}

int pa_write(void* data, int32_t count) {
    PaError err;
    err = Pa_WriteStream(stream, data, count);
    return 0;
}

int pa_done() {
    PaError err;
    err = Pa_StopStream(stream);
    if (err != paNoError) return 1;

    // deinit portaudio
    err = Pa_CloseStream(stream);
    if (err != paNoError) return 1;

    Pa_Terminate();
    return 0;
}

// fixme!
// ------------
int  bars       [4 * 18] = { 0 };
struct bar_keyofs_t {
    int value;
    int limit;
    int dec;
};
bar_keyofs_t bars_keyofs[18] = { 0 };
int  key[18];

void draw_vu() {
    // draw
    int xx = (80 - 72)/2;
    int yy = 32;
    int* bar = bars;
    int* k = key;
    bar_keyofs_t *bar_keyofs = bars_keyofs;
    for (int ch = 0; ch < 18; ch++) {
        for (int op = 0; op < 4; op++) {
            // get height
            int len = *bar + bar_keyofs->value - 12;
            if (len > 63) len = 63;
            if (len < 0)  len = 0;
            len >>= 1;

            // draw bars
            CHAR_INFO* p = (CHAR_INFO*)console.buffer + (console.bufsize.X * yy) + xx;
            for (int i = 0; i < len; i++) {
                p->Char.AsciiChar = '#';
                p->Attributes = *k == 3 ? 0x0F : 0x07;
                p -= console.bufsize.X;
            }
            bar++;

            // advance
            xx++;
        }
        // advance bars
        bar_keyofs->value -= bar_keyofs->dec;
        if (bar_keyofs->value < bar_keyofs->limit) bar_keyofs->value = bar_keyofs->limit;
        bar_keyofs++;
        k++;
    }
}

int main(int argc, char* argv[])
{
    bool render_to_wave = (argc >= 3);

    ESFM_init(&esfm);           // init
    ESFM_write_port(&esfm, 2, 0x05); // set native mode
    ESFM_write_port(&esfm, 3, 0x81); // set native mode

    if (!render_to_wave) pa_init();
    console_open();

    FILE* f = fopen(argv[1], "rb");
    if (f == NULL) {
        printf("error: unable to open file!\n");
        return 1;
    }

    opmctx.io.type = OPMPLAY_IO_FILE;
    opmctx.io.io = f;

    int rtn;

    if ((rtn = opmplay_init(&opmctx.opm)) != OPMPLAY_ERR_OK) {
        printf("unable to init OPMPlay (error = %d)\n", rtn);
        return 1;
    }
    if ((rtn = opmplay_load_header(&opmctx.opm, &opmctx.io)) != OPMPLAY_ERR_OK) {
        printf("unable to load OPM header (error = %d)\n", rtn);
        return 1;
    };
    if ((rtn = opmplay_load_module(&opmctx.opm, &opmctx.io)) != OPMPLAY_ERR_OK) {
        printf("unable to load OPM module (error = %d)\n", rtn);
        return 1;
    };
    opmctx.delay_count = 0;

#if 0
    FILE* logf = fopen("analyze.log", "w");
    for (int i = 0; i < OPMPLAY_MAX_CHANNLES; i++) {
        fprintf(logf, "----channel %i\n", i - 1);
        opm_analyzer(&opmctx.opm, i, logf);
    }
    fclose(logf);
    return 1;
#endif

    // get frame count
    uint32_t frames_loop;
    uint32_t frames_total = opmplay_get_length(&opmctx.opm, &frames_loop);

    std::vector<int16_t> wavedata;

    int ff_pos = 0, ff_counter = 0;
    int16_t buf[FRAMES_PER_BUFFER * CHANNELS] = { 0 };
    while (1) {
        int rtn = synth_render(buf, FRAMES_PER_BUFFER);
        if (render_to_wave) {
            wavedata.insert(wavedata.end(), buf, buf + FRAMES_PER_BUFFER * CHANNELS);
        }
        else {
            pa_write(buf, FRAMES_PER_BUFFER);
        }

        // update bar info
        for (int ch = 0; ch < 18; ch++) {
            int regmask = 1 << 1;
            for (int op = 0; op < 4; op++) {
                if (opmctx.opm.channels[ch + 1].view.regmask & regmask) {
                    // update bars register
                    bars[(ch << 2) + op] = 63 - (opmctx.opm.channels[ch + 1].view.regs.op[op][1] & 0x3F);
                    opmctx.opm.channels[ch + 1].view.regmask &= ~regmask;
                }
                regmask <<= 8;
            }
            key[ch] = opmctx.opm.channels[ch + 1].view.key;
            switch (key[ch] & 3) {
            case 3:
                // on
                bars_keyofs[ch].value = 16;
                bars_keyofs[ch].limit = 0;
                bars_keyofs[ch].dec   = 1;
                break;
            case 2:
                // off
                bars_keyofs[ch].limit = -64;
                bars_keyofs[ch].dec = 1;
                break;
            default: break;
            }
            opmctx.opm.channels[ch + 1].view.key &= 1;
        }

        // update console
        memset(console.buffer, 0, sizeof(CHAR_INFO) * console.bufsize.X * console.bufsize.Y);
        draw_vu();

        {
            int seconds       = opmctx.opm.pos.frame / (0x1234dd / opmctx.opm.header.frame_rate);
            int seconds_total = frames_total         / (0x1234dd / opmctx.opm.header.frame_rate);
            tprintf(0, 0, "%02d:%02d/%02d:%02d [%05d/%05d]", 
                seconds / 60, seconds % 60,
                seconds_total / 60, seconds_total % 60,
                opmctx.opm.pos.frame, frames_total
            );
        }

        console_update();

        if (_kbhit()) {
            _getch();
            break;
        }
    }

    // write wave file
    if (render_to_wave) {
        // create headers
        RIFF_Header riffHeader;
        memcpy(&riffHeader.id, "RIFF", sizeof(riffHeader.id));
        memcpy(&riffHeader.fourcc, "WAVE", sizeof(riffHeader.fourcc));
        riffHeader.size = sizeof(riffHeader.fourcc) + sizeof(fmt_Header) + sizeof(chunk_Header) + (wavedata.size() * sizeof(decltype(wavedata)::value_type));

        fmt_Header fmtHeader;
        memcpy(&fmtHeader.id, "fmt ", sizeof(fmtHeader.id));
        fmtHeader.size = sizeof(fmtHeader) - 8;
        fmtHeader.wFormatTag = 1;           // plain uncompressed PCM
        fmtHeader.nSamplesPerSec = SAMPLE_RATE;
        fmtHeader.nBlockAlign = CHANNELS;
        fmtHeader.nAvgBytesPerSec = SAMPLE_RATE * CHANNELS;
        fmtHeader.nChannels = CHANNELS;
        fmtHeader.wBitsPerSample = 8;

        chunk_Header dataHeader;
        memcpy(&dataHeader.id, "data", sizeof(dataHeader.id));
        dataHeader.size = (wavedata.size() * sizeof(decltype(wavedata)::value_type));

        // write wave file
        FILE* outfile = fopen("out.wav", "wb");

        fwrite(&riffHeader, sizeof(riffHeader), 1, outfile);
        fwrite(&fmtHeader, sizeof(fmtHeader), 1, outfile);
        fwrite(&dataHeader, sizeof(dataHeader), 1, outfile);
        fwrite(wavedata.data(), (wavedata.size() * sizeof(decltype(wavedata)::value_type)), 1, outfile);

        fclose(outfile);
    } else
    pa_done();

    console_done();

    return 0;
}
