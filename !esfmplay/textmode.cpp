#include <stdint.h>
#include <conio.h>
#include <stdio.h>
#include <stdarg.h>
#include "textmode.h"

// -----------------------------------------

static uint16_t *textbuf = (uint16_t*)0xB8000;

void set_textbuf(uint16_t* ptr) {
    textbuf = ptr;
}

// clear rect
void clearect(unsigned long x, unsigned long y, unsigned long w, unsigned long h, unsigned char attr) {
    unsigned long  corr  = (80 - w);
    unsigned short *p    = textbuf + (80 * y) + x;
    
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            *p++ = (attr << 8) | 0x20;
        }
        p += corr;
    }
}

// draw plain string
void drawstring(const char *str, unsigned long x, unsigned long y, unsigned char attr) {
    unsigned short *p = textbuf + (80 * y) + x;
    
    while (*str != '\0') *p++ = (*str++ | (attr << 8));
}    

// draw string with attributes
// '\0' - end, '\xFF\xaa' - set attribute byte 'aa'
void drawastring(const char *str, unsigned long x, unsigned long y) {
    unsigned short *p = textbuf + (80 * y) + x;
    
    unsigned short attr = 0x07 << 8;
    
    while (*str != '\0') if (*str == '\xFF') {attr = (*++str << 8); str++;} else *p++ = (*str++ | attr);
}

// printf 
int tprintf(uint32_t x, uint32_t y, const char *format, ...) {
    char buffer[1024];      // large enough
    va_list arglist;

    va_start( arglist, format );
    int rtn = vsnprintf(buffer, sizeof(buffer), format, arglist);
    drawastring(buffer, x, y);
    va_end( arglist );

    return rtn;
};
