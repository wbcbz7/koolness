#ifndef __FXMATH_H
#define __FXMATH_H

#include <stdlib.h>
#include <math.h>

#ifndef min
#define min(a, b)      ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)      ((a) > (b) ? (a) : (b))
#endif

#define sgn(a)         ((a) < (0) ? (-1) : ((a) > (0) ? (1) : (0)))
#define clamp(a, l, h) ((a) > (h) ? (h) : ((a) < (l) ? (l) : (a)))

#define ee 10E-8
#define sqr(a) ((a)*(a))
#define pi 3.141592653589793f

// upside-down implementation of smoothstep()
inline float smoothstep(float edge0, float edge1, float x) {
  return (edge0 + (x * x * (3 - 2 * x)) * (edge1 - edge0));
}

/*
long abs(long a);
#pragma aux abs = \
    "mov    edx, eax" \
    "sar    edx, 31"  \
    "xor    eax, edx" \
    "sub    eax, edx" \
    parm [eax] value [eax] modify [eax edx]
*/
// fatmap2 ripoff :]

inline long ceilx(long a) {return (a + 0xFFFF) >> 16;}

long imul16(long x, long y);        // (x * y) >> 16
#pragma aux imul16 = \
    " imul  edx        "\
    " shrd  eax,edx,16 "\
    parm [eax] [edx] value [eax]


long imul14(long x, long y);        // (x * y) >> 14
#pragma aux imul14 = \
    " imul  edx        "\
    " shrd  eax,edx,14 "\
    parm [eax] [edx] value [eax]


long idiv16(long x, long y);        // (x << 16) / y
#pragma aux idiv16 = \
    " mov   edx,eax    "\
    " sar   edx,16     "\
    " shl   eax,16     "\
    " idiv  ebx        "\
    parm [eax] [ebx] modify exact [eax edx] value [eax]

long imuldiv(long x, long y, long z);   // (x * y) / z, 64 bit precision
#pragma aux imuldiv = \
    " imul  ebx "     \
    " idiv  ecx "     \
    parm [eax] [ebx] [ecx] modify exact [eax edx] value [eax]
    
// *dst = (long) src;
void fist(long * dst, double src);
#pragma aux fist = \
    "   fistp  dword ptr [eax]  "\
    parm [eax] [8087] modify [8087]
   
extern "C" {
    // float->int FADD trickery
    extern volatile long long _fadd_temp;
    extern const float     _fadd_magic_32_0;
    extern const float     _fadd_magic_16_16;
    extern const float     _fadd_magic_24_8;
    extern const float     _fadd_magic_8_24;
}

long fistf(double src);
#pragma aux fistf = \
    " fadd  dword ptr [_fadd_magic_32_0] " \
    " fstp  qword ptr [_fadd_temp] " \
    " mov   eax, dword ptr [_fadd_temp] " \
    parm [8087] value [eax] modify [8087]
    
long fistfx(double src);
#pragma aux fistfx = \
    " fadd  dword ptr [_fadd_magic_16_16] " \
    " fstp  qword ptr [_fadd_temp] " \
    " mov   eax, dword ptr [_fadd_temp] " \
    parm [8087] value [eax] modify [8087]
    
long fistfx8(double src);
#pragma aux fistfx8 = \
    " fadd  dword ptr [_fadd_magic_24_8] " \
    " fstp  qword ptr [_fadd_temp] " \
    " mov   eax, dword ptr [_fadd_temp] " \
    parm [8087] value [eax] modify [8087]
    
long fistfxtex(double src);
#pragma aux fistfxtex = \
    " fadd  dword ptr [_fadd_magic_8_24] " \
    " fstp  qword ptr [_fadd_temp] " \
    " mov   eax, dword ptr [_fadd_temp] " \
    parm [8087] value [eax] modify [8087]
   
#endif   
    