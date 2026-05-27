#pragma once
/*
    P6Cache - Pentium II/III/Celeron L2 Cache management utility
    --wbcbz7 o9.o7.2o22
    
    p6cache.cpp, mycpuid.cpp/mycpuid.h and lowlevel.h are licensed under following terms:

    Copyright (c) 2022 Artem Vasilev - wbcbz7

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    l2_cache.c and all files from "coreboot" folder are derived from coreboot project
    and licensed under GNU GPL 2.0 terms
*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// read segment registers
uint32_t CS();
#pragma aux CS = "mov eax, cs" value [eax]
uint32_t DS();
#pragma aux DS = "mov eax, ds" value [eax]
uint32_t ES();
#pragma aux ES = "mov eax, es" value [eax]
uint32_t SS();
#pragma aux SS = "mov eax, ss" value [eax]
uint32_t FS();
#pragma aux FS = "mov eax, fs" value [eax]
uint32_t GS();
#pragma aux GS = "mov eax, gs" value [eax]

// write segment register (danger!)
void write_DS(uint32_t);
#pragma aux write_DS = "mov ds, eax" parm [eax]
void write_ES(uint32_t);
#pragma aux write_ES = "mov es, eax" parm [eax]
void write_SS(uint32_t);
#pragma aux write_SS = "mov ss, eax" parm [eax]
void write_FS(uint32_t);
#pragma aux write_FS = "mov fs, eax" parm [eax]
void write_GS(uint32_t);
#pragma aux write_GS = "mov gs, eax" parm [eax]

// read control registers
uint32_t CR0();
#pragma aux CR0 = "mov eax, cr0" value [eax]
uint32_t CR2();
#pragma aux CR2 = "mov eax, cr2" value [eax]
uint32_t CR3();
#pragma aux CR3 = "mov eax, cr3" value [eax]
uint32_t CR4();
#pragma aux CR4 = "mov eax, cr4" value [eax]

// write control registers
void write_CR0(uint32_t);
#pragma aux write_CR0 = "mov cr0, eax" parm [eax]
void write_CR2(uint32_t);
#pragma aux write_CR2 = "mov cr2, eax" parm [eax]
void write_CR3(uint32_t);
#pragma aux write_CR3 = "mov cr3, eax" parm [eax]
void write_CR4(uint32_t);
#pragma aux write_CR4 = "mov cr4, eax" parm [eax]

#pragma pack(push, 1)
typedef struct {
    uint16_t    limit;          // size - 1
    void*       base;           // linear pointer to descriptor table
} descriptorTableAddr;
#pragma pack(pop)

// read system descriptor registers
void sgdt(void* addr);
#pragma aux sgdt = "sgdt dword ptr [eax]" parm [eax]
void sidt(void* addr);
#pragma aux sidt = "sidt dword ptr [eax]" parm [eax]
uint32_t sldt();
#pragma aux sldt = "sldt ax" value [eax]
uint32_t str();
#pragma aux str = "str ax" value [eax]

// write system descriptor registers (ABSOLUTE DANGER!)
// fixme: do via self-modifying code
/*
void lgdt(void* addr);
#pragma aux lgdt = "lgdt dword ptr [eax]" parm [eax]
void lidt(void* addr);
#pragma aux lidt = "lidt dword ptr [eax]" parm [eax]
void lldt(uint32_t);
*/
#pragma aux lldt = "lldt ax" parm [eax]
void ltr(uint32_t);
#pragma aux ltr = "ltr ax" parm [eax]

uint64_t rdtsc();
#pragma aux rdtsc = "rdtsc" value [edx eax]

uint32_t cpuid_eax(uint32_t leaf);
#pragma aux cpuid_eax = "cpuid" parm [eax] value [eax]

#ifdef __cplusplus
}
#endif
