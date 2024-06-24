#pragma once

#include "3d.h"

// polygon 3d clipping

// clip plane definition
struct clipPlane {
    vec4f o, n;
};

// clip against arbitrary plane
// returns number of new faces
size_t clipfaceplane(vec4f *dst, vec4f *src, size_t len, vec4f o, vec4f n);

vec4f setclipper(float fov, float xres, float yres);
int setupClipPlanes5(float znear, vec4f clip, clipPlane *planes);

// clip against each plane
size_t clipface(vec4f *dst, vec4f *src, int numVertices, clipPlane *planes, int numPlanes);


