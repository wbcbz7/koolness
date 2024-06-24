#include "palerp.h"

void pal_lerp(argb32 *dst, argb32 *col0, argb32 *col1, int colors, int steps) {
    for (int i = 0; i < steps+1; i++) {
        argb32 *c0 = col0;
        argb32 *c1 = col1;
        for (int c = 0; c < colors; c++) {
            int r = ((c0->r * steps) + ((((c1->r) - (c0->r)) * i))); 
            int g = ((c0->g * steps) + ((((c1->g) - (c0->g)) * i))); 
            int b = ((c0->b * steps) + ((((c1->b) - (c0->b)) * i))); 
            dst->r = r / steps;
            dst->g = g / steps;
            dst->b = b / steps;
            dst++;
            c0++; c1++;
        }
    }
}

// calcualte color-graded palette
void pal_calc(ptc_palette *dst, argb32 c0, argb32 c1, vec3f cpow, int start, int steps, bool saturate_white) {
    vec3f cf0, cf1;
    cf0.x = c0.r/255.0; cf0.y = c0.g/255.0; cf0.z = c0.b/255.0;
    cf1.x = c1.r/255.0; cf1.y = c1.g/255.0; cf1.z = c1.b/255.0;

    ptc_argb32 *d = dst->data + start;
    float t = 0;
    for (int i = 0; i < steps; i++, t += (1.0 / steps)) {
        vec3f out;
        out.x = pow(cf0.x + t * (cf1.x - cf0.x), cpow.x);
        out.y = pow(cf0.y + t * (cf1.y - cf0.y), cpow.y);
        out.z = pow(cf0.z + t * (cf1.z - cf0.z), cpow.z);

        // cce saturate
        // https://30fps.net/pages/post-processing/
        float maxcomp = max(0.3*out.x, max(0.5*out.y, 0.2*out.z));
        maxcomp = 0.2*maxcomp*maxcomp;
        out.x = out.x + maxcomp;
        out.y = out.y + maxcomp;
        out.z = out.z + maxcomp;

        // "tonemap" the result
        d->r = min(out.x, 1.0) * 255;
        d->g = min(out.y, 1.0) * 255;
        d->b = min(out.z, 1.0) * 255;
        d++;
    }
}

void pal_fade(ptc_palette *dst, ptc_palette *src, argb32 col, int32_t step, int start, int length) {
    int r = (col.r * (255 - step));
    int g = (col.g * (255 - step));
    int b = (col.b * (255 - step));

    ptc_argb32 *d = dst->data + start;
    ptc_argb32 *s = src->data + start;
    for (int i = 0; i < length; i++) {
        d->r = ((s->r * step) + r) >> 8;
        d->g = ((s->g * step) + g) >> 8;
        d->b = ((s->b * step) + b) >> 8;
        s++; d++;
    }
}
