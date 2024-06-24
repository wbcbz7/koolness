#pragma once
#include <stdint.h>

void blink(unsigned long);
#pragma aux blink parm [ebx] modify [eax] = "mov ax, 0x1003" "xor bh, bh" "int 0x10"

unsigned long getcurpos();
#pragma aux getcurpos value [edx] modify [eax ebx] = "mov ah, 3" "mov bh, 0" "int 0x10" "movzx edx, dx"

void setcurpos(unsigned long x, unsigned long y);
#pragma aux setcurpos parm [edx] [eax] modify [eax ebx] = "mov dh, al" "mov ah, 2" "mov bh, 0" "int 0x10"

void setcurshape(unsigned long);
#pragma aux setcurshape parm [ecx] modify [eax] = "mov ax, 0x0103" "int 0x10"

void scrollup(unsigned char lines, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char attr); 
#pragma aux scrollup parm [al] [cl] [ch] [dl] [dh] [bh] modify [ebp] = "mov ah, 0x6" "int 0x10" 

inline uint8_t  textgetrows() { return *(uint8_t*) 0x484; }
inline uint16_t textgetcols() { return *(uint16_t*)0x44A - 1; } // compensate for bios variable

void set_textbuf(uint16_t* ptr);
void clearect(unsigned long x, unsigned long y, unsigned long w, unsigned long h, unsigned char attr = 0x07);
void drawstring(const char *str, unsigned long x, unsigned long y, unsigned char attr = 0x07);
void drawastring(const char *str, unsigned long x, unsigned long y);
int tprintf(uint32_t x, uint32_t y, const char *format, ...);
