#pragma once
#include <stdint.h>
#include "vec.h"
#include "fxmath.h"
#include "linedraw.h"

enum poly_style {
    FACE_POINT  = (0 << 24),
    FACE_SPRITE = (1 << 24),
    FACE_LINE   = (2 << 24),
    FACE_POLY   = (3 << 24),
    FACE_TYPE_MASK = ((1 << 8) - 1) << 24,

    // poly/sprite styles
    POLY_STYLE_MOV = LINE_STYLE_MOV,
    POLY_STYLE_AVG = LINE_STYLE_AVG,
    POLY_STYLE_ADD = LINE_STYLE_ADD,
    POLY_STYLE_SUB = LINE_STYLE_SUB,
    POLY_STYLE_ADDS = LINE_STYLE_ADDS,
    POLY_STYLE_SUBS = LINE_STYLE_SUBS,

    POLY_STYLE_DOUBLESIDED  = (1 << 16),
    POLY_STYLE_WIREFRAME    = (1 << 17),
};

#pragma pack(push, 16)

// uv + color
typedef struct {
    float    u, v;           // u/v coords
    void    *t;              // texture pointer
    uint32_t c;              // packed color
} uv;

// triangle face definition
typedef struct {
    uint32_t   a, b, c;     // vertex coordinates indices (normal is derived)
    uint32_t   ua, ub, uc;  // texture coordinates indices
    uint32_t   col;         // color
    uint32_t   style;       // style
} face;

struct vec3bf;
struct vec3bx;

// baked vertex (screen space but z is used for perspective correction)
struct vec3bf {
    vec4f       p; 
    float       u, v;
    uint32_t    c;
    bool        visible;
    
    inline operator vec3bx();
};

// same but 16:16 fixedpoint
struct vec3bx {
    vec4x       p;               // (x,y,0,0) or (x,y,1/z,0)
    int32_t     u, v;            // or u/z and v/z
    uint32_t    c;
    bool        visible;
    
    inline operator vec3bf();
};

inline vec3bf::operator vec3bx() {
    vec3bx r;
    r.p = p;
    r.u = fistfx(u);
    r.v = fistfx(v);
    r.c = c; r.visible = visible;
    return r;
}

inline vec3bx::operator vec3bf() {
    vec3bf r;
    r.p = p;
    r.u = (u * (1 / 65536.0f)); r.v = (u * (1 / 65536.0f));
    r.c = c; r.visible = visible;
    return r;
}

// triangle addressing list
struct face_idx_t {
    union {
        struct {int32_t a, b, c;};
        int32_t i[3];
    };
};

// triangle drawing linked list
// actulally not even triangle! :D
struct facelist_t {
    vec3bx      v[3];           // baked vertices (can be corrupted!)
    
    uint32_t    style;          // style
    uint32_t    c;              // face color
    void        *t;             // texture pointer
    signed int  avg_z;          // average z for sorting
    facelist_t  *next;          // pointer to next face, 0 - end
};

#pragma pack(pop)

