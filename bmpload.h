#pragma once

#include <stdint.h>

#ifdef _WIN32

// grrrrrrrrrrrrr......

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#else
// ripped from wingdi.h blabla

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER {
    uint16_t    bfType;
    uint32_t    bfSize;
    uint16_t    bfReserved1;
    uint16_t    bfReserved2;
    uint32_t    bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    uint32_t    biSize;
    int32_t     biWidth;
    int32_t     biHeight;
    uint16_t    biPlanes;
    uint16_t    biBitCount;
    uint32_t    biCompression;
    uint32_t    biSizeImage;
    int32_t     biXPelsPerMeter;
    int32_t     biYPelsPerMeter;
    uint32_t    biClrUsed;
    uint32_t    biClrImportant;
} BITMAPINFOHEADER;

// GRRRRRRRRRRRRRRRRRRRRRRRRRR

typedef struct tagCIEXYZ {
    int32_t ciexyzX;            // 2.30fx
    int32_t ciexyzY;
    int32_t ciexyzZ;
} CIEXYZ;

typedef struct tagICEXYZTRIPLE {
    CIEXYZ ciexyzRed;
    CIEXYZ ciexyzGreen;
    CIEXYZ ciexyzBlue;
} CIEXYZTRIPLE;

// BITMAPINFOHEADER::biCompression flags
enum {
    BI_RGB = 0,
    BI_RLE8 = 1,
    BI_RLE4 = 2,
    BI_BITFIELDS = 3,
    BI_JPEG = 4,
    BI_PNG = 5,
    BI_ALPHABITFIELDS = 6,
};

#endif

typedef struct {
    uint32_t        bV5Size;
    int32_t         bV5Width;
    int32_t         bV5Height;
    uint16_t        bV5Planes;
    uint16_t        bV5BitCount;
    uint32_t        bV5Compression;
    uint32_t        bV5SizeImage;
    int32_t         bV5XPelsPerMeter;
    int32_t         bV5YPelsPerMeter;
    uint32_t        bV5ClrUsed;
    uint32_t        bV5ClrImportant;
    uint32_t        bV5RedMask;
    uint32_t        bV5GreenMask;
    uint32_t        bV5BlueMask;
    uint32_t        bV5AlphaMask;
    uint32_t        bV5CSType;
    CIEXYZTRIPLE bV5Endpoints;
    uint32_t        bV5GammaRed;
    uint32_t        bV5GammaGreen;
    uint32_t        bV5GammaBlue;
    uint32_t        bV5Intent;
    uint32_t        bV5ProfileData;
    uint32_t        bV5ProfileSize;
    uint32_t        bV5Reserved;
} BITMAPV5HEADER;

#pragma pack(pop)

// 32bpp
int bmp_load_header(const char *fname, BITMAPV5HEADER *head);
int bmp_load_data(const char *fname, BITMAPV5HEADER *head, uint32_t *buf, int order, uint32_t pitch = 0);

// 15/16bpp
int bmp_load16_header(const char *fname, BITMAPV5HEADER *head);
int bmp_load16_data(const char *fname, BITMAPV5HEADER *head, uint16_t *buf, int order, uint32_t pitch = 0);

// 8bpp
int bmp_load8_header(const char *fname, BITMAPV5HEADER *head);
int bmp_load8_data(const char *fname, BITMAPV5HEADER *head, uint8_t *buf, uint32_t *pal, int order, uint32_t pitch = 0);
