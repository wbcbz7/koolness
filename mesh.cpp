#include "mesh.h"
#include "3dclip.h"

static int   mesh_xres, mesh_yres;
static float mesh_fov;

void mesh_load(mesh_t &mesh, const char *filename)
{
    mesh.fobj = fast_obj_read(filename);

    // allocate memory
    mesh.p.resize(mesh.fobj->position_count);
    mesh.pt.resize(mesh.fobj->position_count);
    mesh.p2d.resize(mesh.fobj->position_count);

    // copy mesh definitions
    float *pp = mesh.fobj->positions;
    for (int i = 0; i < mesh.fobj->position_count; i++) {
        mesh.p[i].x = *pp++;
        mesh.p[i].y = *pp++;
        mesh.p[i].z = *pp++;
        mesh.p[i].w = i;            // marker
    }

    // iterate groups
    mesh.f.resize(mesh.fobj->face_count);
    mesh.nf.resize(mesh.fobj->face_count);
    fastObjIndex* faceptr = mesh.fobj->indices;
    for (int i = 0; i < mesh.fobj->face_count; i++) {
        mesh.f[i].a = faceptr[0].p;
        mesh.f[i].b = faceptr[1].p;
        mesh.f[i].c = faceptr[2].p;
        faceptr += 3;
    }

    // destroy fobj
    fast_obj_destroy(mesh.fobj);
}

void mesh_transform(mesh_t &mesh, mat4 &m) {
    for (int i = 0; i < mesh.p.size(); i++) {
        mesh.pt[i] = mulf(mesh.p[i], m);
    }
}

void mesh_project(mesh_t &mesh, float fov, int xres, int yres) {
    for (int i = 0; i < mesh.pt.size(); i++) {
        proj(mesh.p2d[i], mesh.pt[i], fov, xres, yres);
    }
}

void mesh_calc_nomals(mesh_t &mesh) {
    face_idx_t *fidx  = &mesh.f[0];
    vec4f      *nfidx = &mesh.nf[0];
    for (int i = 0; i < mesh.fobj->face_count; i++) {
        vec4f n = norm(cross(mesh.p[fidx->c] - mesh.p[fidx->a],
                             mesh.p[fidx->c] - mesh.p[fidx->b]));
        *nfidx++ = n; fidx++;
    }
}

void mesh_setup_draw(int xres, int yres, float fov) {
    mesh_xres = xres;
    mesh_yres = yres;
    mesh_fov = fov;
}

// draw without 3D clipping (DANGER - use only if you know what you are doing!)
void mesh_draw(std::vector<facelist_t> &facelist, std::vector<zdata_t> &zmap, mesh_t &mesh, mat4 &nm, clipPlane *clipPlanes, int mincolor, int maxcolor, vec4f &l, float lexp) {
    // draw mesh (TODO: optimize the fuck!)
    face_idx_t  *fidx  = &mesh.f[0];
    vec4f       *nfidx = &mesh.nf[0];
    static vec4f *clipsrc[3];
    for (int i = 0; i < mesh.f.size(); i++) {
        facelist_t fl;
        fl.style = mesh.style;

        // back face culling
        vec4f n = mulf(*nfidx, nm);
        float bf = dot(mesh.pt[fidx->a], n);

        if ((fl.style & POLY_STYLE_DOUBLESIDED) || (bf < 0.0)) {
            // clip
            if (bf > 0) {
                n = -n;     // invert normal
                clipsrc[0] = &mesh.pt[fidx->a];
                clipsrc[1] = &mesh.pt[fidx->c];
                clipsrc[2] = &mesh.pt[fidx->b];
            } else {
                clipsrc[0] = &mesh.pt[fidx->a];
                clipsrc[1] = &mesh.pt[fidx->b];
                clipsrc[2] = &mesh.pt[fidx->c];
            }

            // in frustum
            // calculate lighting
            float r = clamp(dot(l, n), 0.0, 1.0);
            float rr = r;
            for (int i = 0; i < lexp-1; i++) r *= rr;

            fl.c = mincolor + (r * (maxcolor - mincolor));
            vec2f pr = mesh.p2d[clipsrc[0]->w];
            fl.v[0].p.x = fistfx(pr.x);
            fl.v[0].p.y = fistfx(pr.y);

            pr = mesh.p2d[clipsrc[1]->w];
            fl.v[1].p.x = fistfx(pr.x);
            fl.v[1].p.y = fistfx(pr.y);

            pr = mesh.p2d[clipsrc[2]->w];
            fl.v[2].p.x = fistfx(pr.x);
            fl.v[2].p.y = fistfx(pr.y);

            facelist.push_back(fl);
            zdata_t avgz = {facelist.size()-1, (clipsrc[0]->z + clipsrc[1]->z + clipsrc[2]->z) * 21845}; // (65536 / 3)
            zmap.push_back(avgz);
        }

        fidx++; nfidx++;
    }
}

void mesh_draw_clip(std::vector<facelist_t> &facelist, std::vector<zdata_t> &zmap, mesh_t &mesh, mat4 &nm, clipPlane *clipPlanes, int mincolor, int maxcolor, vec4f &l, float lexp) {
    // draw mesh (TODO: optimize the fuck!)
    face_idx_t  *fidx  = &mesh.f[0];
    vec4f       *nfidx = &mesh.nf[0];
    static vec4f clipsrc[3*5], clipdst[3*5];
    for (int i = 0; i < mesh.f.size(); i++) {
        facelist_t fl;
        fl.style = mesh.style;

        // back face culling
        vec4f n = mulf(*nfidx, nm);
        float bf = dot(mesh.pt[fidx->a], n);

        if ((fl.style & POLY_STYLE_DOUBLESIDED) || (bf < 0.0)) {
            // clip
            if (bf > 0) {
                n = -n;     // invert normal
                clipsrc[0] = mesh.pt[fidx->a];
                clipsrc[1] = mesh.pt[fidx->c];
                clipsrc[2] = mesh.pt[fidx->b];
            } else {
                clipsrc[0] = mesh.pt[fidx->a];
                clipsrc[1] = mesh.pt[fidx->b];
                clipsrc[2] = mesh.pt[fidx->c];
            }

            size_t num = clipface(&clipdst[0], &clipsrc[0], 3, clipPlanes, 5);

            if (num >= 3) {
                // in frustum
                // calculate lighting
                float r = clamp(dot(l, n), 0.0, 1.0);
                float rr = r;
                for (int i = 0; i < lexp-1; i++) r *= rr;

                fl.c = mincolor + (r * (maxcolor - mincolor));
                for (int j = 0; j < (num-2); j++) {
                    vec2f pr = (clipdst[0].w < 0) ? proj(clipdst[0], mesh_fov, mesh_xres, mesh_yres) : mesh.p2d[clipdst[0].w];
                    fl.v[0].p.x = fistfx(pr.x);
                    fl.v[0].p.y = fistfx(pr.y);

                    pr = (clipdst[j+1].w < 0) ? proj(clipdst[j+1], mesh_fov, mesh_xres, mesh_yres) : mesh.p2d[clipdst[j+1].w];
                    fl.v[1].p.x = fistfx(pr.x);
                    fl.v[1].p.y = fistfx(pr.y);

                    pr = (clipdst[j+2].w < 0) ? proj(clipdst[j+2], mesh_fov, mesh_xres, mesh_yres) : mesh.p2d[clipdst[j+2].w];
                    fl.v[2].p.x = fistfx(pr.x);
                    fl.v[2].p.y = fistfx(pr.y);

                    facelist.push_back(fl);
                    zdata_t avgz = {facelist.size()-1, (clipdst[0].z + clipdst[j+1].z + clipdst[j+2].z) * 21845}; // (65536 / 3)
                    zmap.push_back(avgz);
                }

            }
        }

        fidx++; nfidx++;
    }
}
