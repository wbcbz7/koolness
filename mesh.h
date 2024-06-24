#pragma once
#include <stdint.h>
#include <vector>
#include "vec.h"
#include "matrix.h"
#include "fxmath.h"
#include "linedraw.h"
#include "poly.h"
#include "fast_obj.h"
#include "3dsort.h"
#include "3dclip.h"

struct mesh_t {
    fastObjMesh *fobj;

    // poly style
    uint32_t                    style;

    // points
    std::vector<vec4f>          p;

    // transformed points
    std::vector<vec4f>          pt;

    // projected points
    std::vector<vec2f>          p2d;

    // face indices
    std::vector<face_idx_t>     f;

    // line indices
    std::vector<vec2i>          l;

    // face normals (flat)
    std::vector<vec4f>          nf;
};

// load mesh
void mesh_load(mesh_t &mesh, const char *filename);

// calculate face normals
void mesh_calc_nomals(mesh_t &mesh);

// transform mesh
void mesh_transform(mesh_t &mesh, mat4 &m);

// project mesh vertices
void mesh_project(mesh_t &mesh, float fov, int xres, int yres);

// setup mesh draw
void mesh_setup_draw(int xres, int yres, float fov);

// draw without 3D clipping (DANGER - use only if you know what you are doing!)
void mesh_draw(std::vector<facelist_t> &facelist, std::vector<zdata_t> &zmap, mesh_t &mesh, mat4 &nm, clipPlane *clipPlanes, int mincolor, int maxcolor, vec4f &l, float lexp);

// draw with 3D clipping
void mesh_draw_clip(std::vector<facelist_t> &facelist, std::vector<zdata_t> &zmap, mesh_t &mesh, mat4 &nm, clipPlane *clipPlanes, int mincolor, int maxcolor, vec4f &l, float lexp);

