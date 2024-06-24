#include <stdio.h>
#include <string.h>

#include "bmpload.h"

int bmp_load8_header(const char * fname, BITMAPV5HEADER * head)
{
    FILE *f;

    if (head == NULL) return 1;
    memset(head, 0, sizeof(BITMAPV5HEADER));

    // open file
    f = fopen(fname, "rb");
    if (!f) return 1;

    // read file header
    BITMAPFILEHEADER bmfile;
    if ((fread(&bmfile, sizeof(bmfile), 1, f) != 1) ||
        (bmfile.bfType != 0x4D42) || 
        (bmfile.bfOffBits > bmfile.bfSize)) {
        fclose(f); return 1;
    } 

    // read info header size (to determine header size)
    uint32_t infoHeaderSize;
    if (fread(&infoHeaderSize, sizeof(infoHeaderSize), 1, f) != 1) {
        fclose(f); return 1;
    }

    // step back and read header
    fseek(f, -sizeof(infoHeaderSize), SEEK_CUR);
    if ((fread(head, sizeof(BITMAPV5HEADER), 1, f) != 1) ||
        (head->bV5Width <= 0) || (head->bV5Planes != 1) || (head->bV5BitCount != 8) || (head->bV5Compression != BI_RGB)) {
        fclose(f); return 1;
    }

    fclose(f);

    return 0;
}

// TODO - fix duplicated BITMAPFILEHEADER reading
int bmp_load8_data(const char * fname, BITMAPV5HEADER * head, uint8_t * buf, uint32_t * pal, int order, size_t pitch)
{
    if (head == NULL) return 1;
    FILE *f;

    // open file
    f = fopen(fname, "rb");
    if (!f) return 1;

    // read file header (yes, again)
    BITMAPFILEHEADER bmfile;
    if ((fread(&bmfile, sizeof(bmfile), 1, f) != 1) ||
        (bmfile.bfType != 0x4D42) ||
        (bmfile.bfOffBits > bmfile.bfSize)) {
        fclose(f); return 1;
    }

    // skip header
    fseek(f, head->bV5Size, SEEK_CUR);

    // read palette
    if (pal != NULL) {
        fseek(f, 0, SEEK_CUR);
        // palette is stored as ARGB8888 (except of A = 0), load it as-is
        fread(pal, head->bV5Size < sizeof(BITMAPV5HEADER) ? 256 : head->bV5ClrUsed, sizeof(uint32_t), f);
        if (ferror(f)) { fclose(f); return 1; }
    }

    // read pixel data
    fseek(f, bmfile.bfOffBits, SEEK_SET);

    int raworder = order ^ (head->bV5Height < 0 ? -1 : 0);
    if (pitch == 0) pitch = head->bV5Width;

    uint8_t *p = (raworder ? buf + pitch * (head->bV5Height - 1) : buf);
    signed long srcFixup = (4 - (head->bV5Width & 3) & 3);
    for (int i = 0; i < head->bV5Height; i++) {
        if (fread(p, head->bV5Width, 1, f) != 1) { fclose(f); return 1; }
        if (raworder) p -= pitch; else p += pitch;

        // dword alignment fixup
        fseek(f, srcFixup, SEEK_CUR);
    }

    // fix height sign
    if (head->bV5Height < 0) head->bV5Height = -head->bV5Height;

    return 0;
}
