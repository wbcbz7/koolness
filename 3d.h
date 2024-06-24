#ifndef __3D_H
#define __3D_H

#include "vec.h"
#include "fxmath.h"

// epsilon

inline vec2f proj(vec3f &f, float fov, float xres, float yres) {
    vec2f p;
    
    // does mirror if z < 0!
    
    float t = (fov / (f.z + ee));
    p.x = (f.x * t) + (xres/2);
    p.y = (f.y * t) + (yres/2);
    
    return p;
}

inline vec2f proj(vec4f &f, float fov, float xres, float yres) {
    vec2f p;
    
    // does mirror if z < 0!
    
    float t = (fov / (f.z + ee));
    p.x = (f.x * t) + (xres/2);
    p.y = (f.y * t) + (yres/2);
    
    return p;
}

inline void proj(vec2f& of, vec3f &f, float fov, float xres, float yres) {
    // does mirror if z < 0!
    float t = (fov / (f.z + ee));
    of.x = (f.x * t) + (xres/2);
    of.y = (f.y * t) + (yres/2);
}

inline void proj(vec2f& of, vec4f &f, float fov, float xres, float yres) {
    // does mirror if z < 0!
    float t = (fov / (f.z + ee));
    of.x = (f.x * t) + (xres/2);
    of.y = (f.y * t) + (yres/2);
}


#endif
