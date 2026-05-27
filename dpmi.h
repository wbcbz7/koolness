#ifndef DPMI_H
#define DPMI_H

#include <i86.h>

// some useful DPMI functions
// by wbc\\bz7 zo.oz.zolb

#pragma pack (push, 1)

// dpmi realmode regs structire
typedef struct {
    unsigned long EDI;
    unsigned long ESI;
    unsigned long EBP;
    unsigned long reserved;
    unsigned long EBX;
    unsigned long EDX;
    unsigned long ECX;
    unsigned long EAX;
    unsigned short flags;
    unsigned short ES,DS,FS,GS,IP,CS,SP,SS;
} _dpmi_rmregs;

// dpmi segment:selector pair
typedef struct {
    unsigned short int segment;
    unsigned short int selector;
} _dpmi_ptr;

// dpmi descriptor struct
typedef struct {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_mid;

    // access byte
    unsigned char  accessed : 1;   // set by CPU
    unsigned char  rw : 1;         // read enable bit for code, write enable bit for data
    unsigned char  dc : 1;         // direction (set if grows down) bit for data, "conforming" bit for code (read the docs :)
    unsigned char  code : 1;       // set if code, clear if data
    unsigned char  type : 1;       // set for code/data, clear for special segments
    unsigned char  dpl : 2;        // privilegie level
    unsigned char  present : 1;    // set if present

    unsigned char  limit_high : 4;

    unsigned char  avl : 1;        // available for software needs
    unsigned char  longseg : 1;    // set if 64bit segment
    unsigned char  size : 1;       // set if 32bit, clear if 16/64bit
    unsigned char  limitgran : 1;  // set if limit in pages (4k units), clear if in bytes

    unsigned char  base_high;
} _dpmi_descriptor;

#pragma pack (pop)

void dpmi_getdosmem(int size, _dpmi_ptr *p);
void dpmi_freedosmem(_dpmi_ptr *p);
void dpmi_rminterrupt(int int_num, _dpmi_rmregs *regs);
void *dpmi_mapphysical(unsigned long size, void *p);
void dpmi_unmapphysical(void *p);
int rmint386x(int intnum, union REGS *in, union REGS *out, struct SREGS *seg);
void rmintr(int intnum, union REGPACK *r);

extern unsigned int dpmi_status;

#endif