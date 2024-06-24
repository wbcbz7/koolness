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