#include <vector>

#include "3dclip.h"
#include "3d.h"

size_t clipfaceplane(vec4f *dst, vec4f *src, size_t len, vec4f o, vec4f n) {
    if (len == 0) return 0;
    
    size_t k = (1 % len), r = 0;
    float t, it, t1, t2;
    vec4f v1, v2;
    
    for (size_t i = 0; i < len; i++) {
        v1 = src[i]; v2 = src[k++];  if (k >= len) k = 0;
        t1 = dot((v1 - o), n); t2 = dot((v2 - o), n);
        
        if (t1 >= 0) {
            dst[r++] = v1;
            if (t2 >= 0) continue;
        } else if (t2 < 0) continue;
        
        {
            t = t1 / (t1 - t2);
            it = 1.0f - t;
            dst[r].x = it * v1.x + t * v2.x;
            dst[r].y = it * v1.y + t * v2.y;
            dst[r].z = it * v1.z + t * v2.z;
            dst[r].w = -1;      // mark vertex as "new"
            
            r++;
        }
    }
    
    return r;
}


vec4f setclipper(float fov, float xres, float yres) {

    float ah = atan2(xres/2, fov) - ee;
    float av = atan2(yres/2, fov) - ee;
    float sh = sin(ah);
    float sv = sin(av);
    float ch = cos(ah);
    float cv = cos(av);

    vec4f r = {sh, sv, ch, cv};    
    
    return r;
}

// make clip planes
int setupClipPlanes5(float znear, vec4f clip, clipPlane *planes) {
    // z clip
    planes[0].o.x = 0;
    planes[0].o.y = 0;
    planes[0].o.z = znear + ee;

    planes[0].n.x = 0;
    planes[0].n.y = 0;
    planes[0].n.z = 1;

    // side clip
    planes[1].o.x = planes[1].o.y = planes[1].o.z = 0;
    planes[1].n.x =  clip.z; planes[1].n.y = 0; planes[1].n.z = clip.x;

    planes[2].o.x = planes[2].o.y = planes[1].o.z = 0;
    planes[2].n.x = -clip.z; planes[2].n.y = 0; planes[2].n.z = clip.x;

    planes[3].o.x = planes[3].o.y = planes[3].o.z = 0;
    planes[3].n.x = 0; planes[3].n.y =  clip.w; planes[3].n.z = clip.y;

    planes[4].o.x = planes[4].o.y = planes[4].o.z = 0;
    planes[4].n.x = 0; planes[4].n.y = -clip.w; planes[4].n.z = clip.y;

    return 0;
}

// clip against each plane
size_t clipface(vec4f *dst, vec4f *src, int numVertices, clipPlane *planes, int numPlanes) {
    vec4f *s = src, *d = dst, *tmp;
    size_t num = numVertices;
    for (size_t i = 0; i < numPlanes; i++) {
        num = clipfaceplane(d, s, num, planes[i].o, planes[i].n);
        tmp = s; s = d; d = tmp;
    }
    
    return num;
}


