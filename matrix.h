#ifndef __MATRIX_H
#define __MATRIX_H

#include <math.h>
#include "fxmath.h"
//#include "sintab.h"
#include "vec.h"

// err, yes :(
typedef float mat2[4];
typedef float mat3[9];
typedef float mat4[16];

// --------------------------------

inline void ident2(mat2 &m) {m[0] = m[3] = 1.0f; m[1] = m[2] = 0.0f;}

inline void ident3(mat3 &m) {m[0] = m[4] = m[8] = 1.0f; m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0.0f;}

inline void ident4(mat4 &m) {
    m[0] = m[5] = m[10] = m[15] = 1.0f;
    m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
}

// --------------------------------

inline void scale2(mat2 &m, float x, float y) {
    m[0] = x; m[3] = y; m[1] = m[2] = 0.0f;
}

inline void scale3(mat3 &m, float x, float y, float z) {
    m[0] = x;     m[4] = y;     m[8] = z;
    m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0.0f;
}

inline void scale4(mat4 &m, float x, float y, float z, float w) {
    m[0] = x; m[5] = y; m[10] = z; m[15] = w;
    m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0f;
}

// --------------------------------


inline void ofs3(mat3 &m, float x, float y) {
    ident3(m);
    m[2] = x; m[5] = y;
}

inline void ofs3_imm(mat3 &m, float x, float y) {
    m[2] = x; m[5] = y;
}

inline void ofs3_imm_d(mat3 &m, float x, float y) {
    m[2] += x; m[5] += y;
}

inline void ofs4(mat4 &m, float x, float y, float z) {
    ident4(m);
    m[3] = x; m[7] = y; m[11] = z;
}

inline void ofs4_imm(mat4 &m, float x, float y, float z) {
    m[3] = x; m[7] = y; m[11] = z;
}

inline void ofs4_imm_d(mat4 &m, float x, float y, float z) {
    m[3] += x; m[7] += y; m[11] += z;
}


// --------------------------------

inline void rot2(mat2 &m, float a) {
    m[0] = m[3] = cos(a); m[1] = -sin(a); m[2] = sin(a);
}

/*
inline void rot2t(mat2 &m, unsigned long a) {
    m[0] = m[3] = costab[a & 0xFFFF]; m[1] = -sintab[a & 0xFFFF]; m[2] = sintab[a & 0xFFFF];
}
*/

// xyz order
inline void rot3(mat3 &m, float ax, float ay, float az) {
    float  a = cos(ax), b = sin(ax), c = cos(ay),
           d = sin(ay), e = cos(az), f = sin(az);
    float ad = a * d,   bd = b * d;
    
    m[0] = c * e; m[1] = -c * f; m[2] = -d;
    m[3] = (-bd * e) + (a * f); m[4] = ( bd * f) + (a * e); m[5] = (-b * c);
    m[6] = ( ad * e) + (b * f); m[7] = (-ad * f) + (b * e); m[8] = ( a * c);
    
}

/*
inline void rot3t(mat3 &m, unsigned long ax, unsigned long ay, unsigned long az) {
    float  a = costab[ax & 0xFFFF], b = sintab[ax & 0xFFFF], c = costab[ay & 0xFFFF],
           d = sintab[ay & 0xFFFF], e = costab[az & 0xFFFF], f = sintab[ax & 0xFFFF];
    float ad = a * d,   bd = b * d;
    
    m[0] = c * e; m[1] = -c * f; m[2] = -d;
    m[3] = (-bd * e) + (a * f); m[4] = ( bd * f) + (a * e); m[5] = (-b * c);
    m[6] = ( ad * e) + (b * f); m[7] = (-ad * f) + (b * e); m[8] = ( a * c);
    
}
*/

inline void rot4(mat4 &m, float ax, float ay, float az) {
    float  a = cos(ax), b = sin(ax), c = cos(ay),
           d = sin(ay), e = cos(az), f = sin(az);
    float ad = a * d,   bd = b * d;
    
    m[0] = c * e; m[1] = -c * f; m[2] = -d;
    m[4] = (-bd * e) + (a * f); m[5] = ( bd * f) + (a * e); m[6]  = (-b * c);
    m[8] = ( ad * e) + (b * f); m[9] = (-ad * f) + (b * e); m[10] = ( a * c);
    
    m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f; m[15] = 1.0f;
    
}

inline void rot4t(mat4 &m, unsigned long ax, unsigned long ay, unsigned long az) {
    float  a = cos(ax), b = sin(ax), c = cos(ay), d = sin(ay), e = cos(az), f = sin(ax);
    float ad = a * d,   bd = b * d;
    
    m[0] = c * e; m[1] = -c * f; m[2] = -d;
    m[4] = (-bd * e) + (a * f); m[5] = ( bd * f) + (a * e); m[6]  = (-b * c);
    m[8] = ( ad * e) + (b * f); m[9] = (-ad * f) + (b * e); m[10] = ( a * c);
    
    m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f; m[15] = 1.0f;
    
}

// zyx order
    /*
    [ ec -fa-edb  fb-eda ]
    [ fc  ea-fdb -eb-fda ]
    [  d    cb      ca   ]
    */
inline void rot3r(mat3 &m, float ax, float ay, float az) {
    float  a = cos(ax), b = sin(ax), c = cos(ay),
           d = sin(ay), e = cos(az), f = sin(az);
    float db = d * b,   da = d * a;
    
    m[0] = e * c; m[1] = -((f * a) + (e * db)); m[2] =   (f * b) - (e * da);
    m[3] = f * c; m[4] =   (e * a) - (f * db);  m[5] = -((e * b) + (f * da));
    m[6] = d;     m[7] =    c * b;              m[8] = c * a;
    
}

/*
inline void rot3rt(mat3 &m, unsigned long ax, unsigned long ay, unsigned long az) {
    float  a = costab[ax & 0xFFFF], b = sintab[ax & 0xFFFF], c = costab[ay & 0xFFFF],
           d = sintab[ay & 0xFFFF], e = costab[az & 0xFFFF], f = sintab[ax & 0xFFFF];
    float ad = a * d,   bd = b * d;
    
    m[0] = c * e; m[1] = -c * f; m[2] = -d;
    m[3] = (-bd * e) + (a * f); m[4] = ( bd * f) + (a * e); m[5] = (-b * c);
    m[6] = ( ad * e) + (b * f); m[7] = (-ad * f) + (b * e); m[8] = ( a * c);
    
}
*/

inline void rot4r(mat4 &m, float ax, float ay, float az) {
    float  a = cos(ax), b = sin(ax), c = cos(ay),
           d = sin(ay), e = cos(az), f = sin(az);
    float ad = a * d,   bd = b * d;
    
    m[0] = c * e; m[1] = -c * f; m[2] = -d;
    m[4] = (-bd * e) + (a * f); m[5] = ( bd * f) + (a * e); m[6]  = (-b * c);
    m[8] = ( ad * e) + (b * f); m[9] = (-ad * f) + (b * e); m[10] = ( a * c);
    
    m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f; m[15] = 1.0f;
    
}

/*
inline void rot4rt(mat4 &m, unsigned long ax, unsigned long ay, unsigned long az) {
    float  a = costab[ax & 0xFFFF], b = sintab[ax & 0xFFFF], c = costab[ay & 0xFFFF],
           d = sintab[ay & 0xFFFF], e = costab[az & 0xFFFF], f = sintab[ax & 0xFFFF];
    float ad = a * d,   bd = b * d;
    
    m[0] = c * e; m[1] = -c * f; m[2] = -d;
    m[4] = (-bd * e) + (a * f); m[5] = ( bd * f) + (a * e); m[6]  = (-b * c);
    m[8] = ( ad * e) + (b * f); m[9] = (-ad * f) + (b * e); m[10] = ( a * c);
    
    m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f; m[15] = 1.0f;
    
}
*/


// -----------------------------

inline vec2f mul(vec2f &lhs, mat2 &rhs) {
    vec2f r;
    r.x = lhs.x*rhs[0] + lhs.y*rhs[1];
    r.y = lhs.x*rhs[2] + lhs.y*rhs[3];
    return r;
}

inline void mul(vec2f &lhs, mat2 &rhs, vec2f &r) {
    float xxx;  // fix feedback
    xxx = lhs.x*rhs[0] + lhs.y*rhs[1];
    r.y = lhs.x*rhs[2] + lhs.y*rhs[3];
    r.x = xxx;
}


inline vec3f mul(vec3f &lhs, mat3 &rhs) {
    vec3f r;
    r.x = lhs.x*rhs[0] + lhs.y*rhs[1] + lhs.z*rhs[2];
    r.y = lhs.x*rhs[3] + lhs.y*rhs[4] + lhs.z*rhs[5];
    r.z = lhs.x*rhs[6] + lhs.y*rhs[7] + lhs.z*rhs[8];
    return r;
}

inline vec4f mul(vec4f &lhs, mat4 &rhs) {
    vec4f r;
    r.x = lhs.x*rhs[0]  + lhs.y*rhs[1]  + lhs.z*rhs[2]  + lhs.w*rhs[3];
    r.y = lhs.x*rhs[4]  + lhs.y*rhs[5]  + lhs.z*rhs[6]  + lhs.w*rhs[7];
    r.z = lhs.x*rhs[8]  + lhs.y*rhs[9]  + lhs.z*rhs[10] + lhs.w*rhs[11];
    r.w = lhs.x*rhs[12] + lhs.y*rhs[13] + lhs.z*rhs[14] + lhs.w*rhs[15];
    return r;
}

// 9 mul
inline vec4f mulf(vec4f &lhs, mat4 &rhs) {
    vec4f r;
    r.x = lhs.x*rhs[0] + lhs.y*rhs[1] + lhs.z*rhs[2]  + rhs[3];
    r.y = lhs.x*rhs[4] + lhs.y*rhs[5] + lhs.z*rhs[6]  + rhs[7];
    r.z = lhs.x*rhs[8] + lhs.y*rhs[9] + lhs.z*rhs[10] + rhs[11];
    r.w = lhs.w;
    return r;
}

// -----------------------------

inline void matmul2(mat2 &r, mat2 &a, mat2 &b) {
    r[0] = (a[0] * b[0]) + (a[1] * b[2]);
    r[1] = (a[0] * b[1]) + (a[1] * b[3]);
    r[2] = (a[2] * b[0]) + (a[3] * b[2]);
    r[3] = (a[2] * b[1]) + (a[3] * b[3]);
}

inline void matmul3(mat3 &r, mat3 &a, mat3 &b) {
    r[0] = (a[0] * b[0]) + (a[1] * b[3]) + (a[2] * b[6]);
    r[1] = (a[0] * b[1]) + (a[1] * b[4]) + (a[2] * b[7]);
    r[2] = (a[0] * b[2]) + (a[1] * b[5]) + (a[2] * b[8]);
    
    r[3] = (a[3] * b[0]) + (a[4] * b[3]) + (a[5] * b[6]);
    r[4] = (a[3] * b[1]) + (a[4] * b[4]) + (a[5] * b[7]);
    r[5] = (a[3] * b[2]) + (a[4] * b[5]) + (a[5] * b[8]);
    
    r[6] = (a[6] * b[0]) + (a[7] * b[3]) + (a[8] * b[6]);
    r[7] = (a[6] * b[1]) + (a[7] * b[4]) + (a[8] * b[7]);
    r[8] = (a[6] * b[2]) + (a[7] * b[5]) + (a[8] * b[8]);
}

inline void matmul4(mat4 &r, mat4 &a, mat4 &b) {
    r[0]  = (a[0]  * b[0]) + (a[1]  * b[4])  + (a[2]  * b[8])  + (a[3]  * b[12]);
    r[1]  = (a[0]  * b[1]) + (a[1]  * b[5])  + (a[2]  * b[9])  + (a[3]  * b[13]);
    r[2]  = (a[0]  * b[2]) + (a[1]  * b[6])  + (a[2]  * b[10]) + (a[3]  * b[14]);
    r[3]  = (a[0]  * b[3]) + (a[1]  * b[7])  + (a[2]  * b[11]) + (a[3]  * b[15]);
    
    r[4]  = (a[4]  * b[0]) + (a[5]  * b[4])  + (a[6]  * b[8])  + (a[7]  * b[12]);
    r[5]  = (a[4]  * b[1]) + (a[5]  * b[5])  + (a[6]  * b[9])  + (a[7]  * b[13]);
    r[6]  = (a[4]  * b[2]) + (a[5]  * b[6])  + (a[6]  * b[10]) + (a[7]  * b[14]);
    r[7]  = (a[4]  * b[3]) + (a[5]  * b[7])  + (a[6]  * b[11]) + (a[7]  * b[15]);
    
    r[8]  = (a[8]  * b[0]) + (a[9]  * b[4])  + (a[10] * b[8])  + (a[11] * b[12]);
    r[9]  = (a[8]  * b[1]) + (a[9]  * b[5])  + (a[10] * b[9])  + (a[11] * b[13]);
    r[10] = (a[8]  * b[2]) + (a[9]  * b[6])  + (a[10] * b[10]) + (a[11] * b[14]);
    r[11] = (a[8]  * b[3]) + (a[9]  * b[7])  + (a[10] * b[11]) + (a[11] * b[15]);
    
    r[12] = (a[12] * b[0]) + (a[13] * b[4])  + (a[14] * b[8])  + (a[15] * b[12]);
    r[13] = (a[12] * b[1]) + (a[13] * b[5])  + (a[14] * b[9])  + (a[15] * b[13]);
    r[14] = (a[12] * b[2]) + (a[13] * b[6])  + (a[14] * b[10]) + (a[15] * b[14]);
    r[15] = (a[12] * b[3]) + (a[13] * b[7])  + (a[14] * b[11]) + (a[15] * b[15]);
}

// --------------------
inline void lookat(mat3 &m, vec3f &eye, vec3f &pos, vec3f &up) {
    vec3f forward = norm(pos - eye);
    vec3f side = norm(cross(forward, up));
    vec3f newup = cross(side, forward);

    m[0] = side.x;     m[3] = side.y;     m[6] = side.z; 
    m[1] = newup.x;    m[4] = newup.y;    m[7] = newup.z; 
    m[2] = -forward.x; m[5] = -forward.y; m[8] = -forward.z; 
}

// inverse
inline void lookatr(mat3 &m, vec3f &eye, vec3f &pos, vec3f &up) {
    vec3f forward = norm(pos - eye);
    vec3f side = norm(cross(forward, up));
    vec3f newup = cross(side, forward);
   

    m[0] = side.x;     m[1] = side.y;     m[2] = side.z; 
    m[3] = newup.x;    m[4] = newup.y;    m[5] = newup.z; 
    m[6] = -forward.x; m[7] = -forward.y; m[8] = -forward.z;
}

inline void lookat4(mat4 &m, vec4f &eye, vec4f &pos, vec4f &up) {
    vec4f forward = norm(pos - eye);
    vec4f side = norm(cross(forward, up));
    vec4f newup = cross(side, forward);

    ident4(m);
    
    m[0] = side.x;     m[4] = side.y;     m[8]  = side.z; 
    m[1] = newup.x;    m[5] = newup.y;    m[9]  = newup.z; 
    m[2] = -forward.x; m[6] = -forward.y; m[10] = -forward.z; 
}

// inverse
inline void lookatr4(mat4 &m, vec4f &eye, vec4f &pos, vec4f &up) {
    vec4f forward = norm(pos - eye);
    vec4f side = norm(cross(forward, up));
    vec4f newup = cross(side, forward);
   
    ident4(m);
   
    m[0] = side.x;     m[1] = side.y;     m[2]  = side.z; 
    m[4] = newup.x;    m[5] = newup.y;    m[6]  = newup.z; 
    m[8] = -forward.x; m[9] = -forward.y; m[10] = -forward.z;
}

#endif
