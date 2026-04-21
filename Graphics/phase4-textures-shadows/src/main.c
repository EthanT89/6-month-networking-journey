/*
 * main.c -- phase 4: textures and shadow mapping
 */

#include "../../shared/framebuffer/framebuffer.h"
#include "../../shared/geometry/geometry.h"
#include "../../shared/lighting/lighting.h"
#include "../../shared/math/camera/camera.h"
#include "../../shared/math/mat/matrix.h"
#include "../../shared/math/projection/projection.h"
#include "../../shared/math/vec/vector.h"
#include "../../shared/raster/raster.h"
#include "./texture/texture.h"
#include "./shadow/shadow.h"

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define FB_WIDTH  800
#define FB_HEIGHT 600
#define CUBE_VERTEX_COUNT 8
#define CUBE_TRI_COUNT    12

#define AMBIENT 0.1f

/* -----------------------------------------------------------------------
 * Scene data — vertex positions and triangle connectivity
 * ----------------------------------------------------------------------- */

static const int cube_tri_indices[CUBE_TRI_COUNT][3] = {
    {4, 5, 6}, {4, 6, 7},   // front  (z=+1)
    {1, 0, 3}, {1, 3, 2},   // back   (z=-1)
    {5, 1, 2}, {5, 2, 6},   // right  (x=+1)
    {0, 4, 7}, {0, 7, 3},   // left   (x=-1)
    {7, 6, 2}, {7, 2, 3},   // top    (y=+1)
    {0, 1, 5}, {0, 5, 4},   // bottom (y=-1)
};

static const float cube_uvs[CUBE_TRI_COUNT][3][2] = {
    {{0,1},{1,1},{1,0}}, {{0,1},{1,0},{0,0}},  // front
    {{0,1},{1,1},{1,0}}, {{0,1},{1,0},{0,0}},  // back
    {{0,1},{1,1},{1,0}}, {{0,1},{1,0},{0,0}},  // right
    {{0,1},{1,1},{1,0}}, {{0,1},{1,0},{0,0}},  // left
    {{0,1},{1,1},{1,0}}, {{0,1},{1,0},{0,0}},  // top
    {{0,1},{1,1},{1,0}}, {{0,1},{1,0},{0,0}},  // bottom
};

/* -----------------------------------------------------------------------
 * Helpers
 * ----------------------------------------------------------------------- */

static struct Vector4 *create_cube_vertices(void) {
    struct Vector4 *cube = malloc(CUBE_VERTEX_COUNT * sizeof(struct Vector4));
    if (!cube) return NULL;
    cube[0] = (struct Vector4){-1,-1,-1,1};
    cube[1] = (struct Vector4){ 1,-1,-1,1};
    cube[2] = (struct Vector4){ 1, 1,-1,1};
    cube[3] = (struct Vector4){-1, 1,-1,1};
    cube[4] = (struct Vector4){-1,-1, 1,1};
    cube[5] = (struct Vector4){ 1,-1, 1,1};
    cube[6] = (struct Vector4){ 1, 1, 1,1};
    cube[7] = (struct Vector4){-1, 1, 1,1};
    return cube;
}

/* -----------------------------------------------------------------------
 * Stage 1 — World space
 * Apply model matrix, compute face normals and vertex normals.
 * Fills v_world on each triangle.
 * ----------------------------------------------------------------------- */

static void stage_world_space(
    const struct Vector4 cube[CUBE_VERTEX_COUNT],
    struct Matrix model,
    struct Vector3 world_verts[CUBE_VERTEX_COUNT],
    struct Triangle triangles[CUBE_TRI_COUNT],
    struct Vector3 vert_norms[CUBE_VERTEX_COUNT])
{
    // model space → world space
    for (int vi = 0; vi < CUBE_VERTEX_COUNT; vi++){
        struct Vector4 w = mat4_mul_vec4(model, cube[vi]);
        world_verts[vi] = (struct Vector3){w.x, w.y, w.z};
    }

    // build triangles in world space, compute face normals
    for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
        triangles[ti].v_world[0] = world_verts[cube_tri_indices[ti][0]];
        triangles[ti].v_world[1] = world_verts[cube_tri_indices[ti][1]];
        triangles[ti].v_world[2] = world_verts[cube_tri_indices[ti][2]];
        triangles[ti].normal = compute_face_normal(
            triangles[ti].v_world[0],
            triangles[ti].v_world[1],
            triangles[ti].v_world[2]);
        triangles[ti].uv[0] = *(struct Vector2*)cube_uvs[ti][0];
        triangles[ti].uv[1] = *(struct Vector2*)cube_uvs[ti][1];
        triangles[ti].uv[2] = *(struct Vector2*)cube_uvs[ti][2];
    }

    // average face normals per vertex → smooth shading normals
    for (int vi = 0; vi < CUBE_VERTEX_COUNT; vi++){
        struct Vector3 sum = {0,0,0};
        for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
            if (cube_tri_indices[ti][0] == vi ||
                cube_tri_indices[ti][1] == vi ||
                cube_tri_indices[ti][2] == vi){
                sum = v3_addition(sum, triangles[ti].normal);
            }
        }
        vert_norms[vi] = v3_norm(sum);
    }
}

/* -----------------------------------------------------------------------
 * Stage 2 — Lighting
 * Compute per-vertex brightness using Phong model.
 * Fills brightness[3] on each triangle.
 * ----------------------------------------------------------------------- */

static void stage_lighting(
    struct Triangle triangles[CUBE_TRI_COUNT],
    const struct Vector3 vert_norms[CUBE_VERTEX_COUNT],
    struct DirectionalLight light,
    struct Vector3 camera_pos)
{
    for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
        for (int j = 0; j < 3; j++){
            int vid = cube_tri_indices[ti][j];
            float diffuse  = compute_diffuse(vert_norms[vid], light);
            float specular = 0.0f;

            if (diffuse > 0.0f){
                struct Vector3 mid      = get_triangle_midpoint(&triangles[ti]);
                struct Vector3 view_dir = v3_norm(v3_subtraction(camera_pos, mid));
                specular = compute_specular(triangles[ti].normal, light.direction, view_dir, 16.0f);
            }

            triangles[ti].brightness[j] = fminf(AMBIENT + diffuse + specular, 1.0f);
        }
    }
}

/* -----------------------------------------------------------------------
 * Stage 3 — Clip / screen space
 * Apply MVP, perspective divide, viewport transform.
 * Fills v_screen and clip_w on each triangle.
 * ----------------------------------------------------------------------- */

static void stage_screen_space(
    const struct Vector4 cube[CUBE_VERTEX_COUNT],
    struct Matrix mvp,
    struct Triangle triangles[CUBE_TRI_COUNT])
{
    struct Vector4 clip_verts[CUBE_VERTEX_COUNT];
    struct Vector3 screen_verts[CUBE_VERTEX_COUNT];

    for (int vi = 0; vi < CUBE_VERTEX_COUNT; vi++){
        clip_verts[vi] = mat4_mul_vec4(mvp, cube[vi]);
        struct Vector4 ndc        = perspective_divide(clip_verts[vi]);
        struct Vector4 viewport_v = viewport(ndc, FB_HEIGHT, FB_WIDTH);
        screen_verts[vi] = (struct Vector3){viewport_v.x, viewport_v.y, viewport_v.z};
    }

    for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
        triangles[ti].v_screen[0] = screen_verts[cube_tri_indices[ti][0]];
        triangles[ti].v_screen[1] = screen_verts[cube_tri_indices[ti][1]];
        triangles[ti].v_screen[2] = screen_verts[cube_tri_indices[ti][2]];
        triangles[ti].clip_w[0]   = clip_verts[cube_tri_indices[ti][0]].w;
        triangles[ti].clip_w[1]   = clip_verts[cube_tri_indices[ti][1]].w;
        triangles[ti].clip_w[2]   = clip_verts[cube_tri_indices[ti][2]].w;
    }
}

/* -----------------------------------------------------------------------
 * Render
 * ----------------------------------------------------------------------- */

static void render_single_cube(void) {
    static struct Framebuffer fb;
    static bool fb_initialized = false;

    if (!fb_initialized){
        fb = fb_create(FB_WIDTH, FB_HEIGHT);
        fb_initialized = true;
    }
    fb_clear(&fb);

    // --- texture ---
    struct Texture texture;
    texture_load("./content/mountains-lake.jpg", &texture);

    // --- geometry ---
    struct Vector4 *cube = create_cube_vertices();
    if (!cube) return;

    // --- transform parameters ---
    struct Matrix translate = translation_constructor(0.0f, 0.0f, 2.0f);
    struct Matrix rotate    = mat4_mul(rotation_x_constructor(3.0f),
                              mat4_mul(rotation_y_constructor(2.1f),
                                       rotation_z_constructor(2.2f)));
    struct Matrix scale     = scale_constructor(2.0f, 2.0f, 2.0f);
    struct Matrix model     = mat4_mul(translate, mat4_mul(rotate, scale));

    // --- light ---
    struct DirectionalLight light;
    light.direction = v3_norm((struct Vector3){-1.0f, -1.0f, 1.0f});
    light.intensity = 1.0f;

    // --- camera ---
    struct Vector3 camera = {-5.0f, 0.0f, 0.0f};
    struct Vector3 target = { 5.0f, 0.0f, 0.0f};
    struct Vector3 up     = { 0.0f, 1.0f, 0.0f};

    // --- single triangle array — carries everything ---
    struct Triangle triangles[CUBE_TRI_COUNT];
    struct Vector3  world_verts[CUBE_VERTEX_COUNT];
    struct Vector3  vert_norms[CUBE_VERTEX_COUNT];

    // stage 1 — world space + normals + UVs
    stage_world_space(cube, model, world_verts, triangles, vert_norms);

    // stage 2 — lighting
    stage_lighting(triangles, vert_norms, light, camera);

    // stage 3 — screen space
    struct Matrix view = lookAt(camera, target, up);
    struct Matrix proj = perspective(1.5708f, (float)FB_WIDTH/FB_HEIGHT, 0.1f, 100.0f);
    struct Matrix mvp  = mat4_mul(proj, mat4_mul(view, model));
    stage_screen_space(cube, mvp, triangles);

    // --- light camera (for shadow map) ---
    struct Vector3 scene_center = {0.0f, 0.0f, 0.0f};

    // --- pass 1: shadow map ---
    struct ShadowMap shadow_map = shadow_map_create();
    shadow_map_render(&shadow_map, world_verts, CUBE_VERTEX_COUNT,
                      cube_tri_indices, CUBE_TRI_COUNT, light, scene_center);

    // --- pass 2: rasterize ---
    for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
        if (is_back_face(&triangles[ti])) continue;
        draw_triangle_textured(&fb,
            triangles[ti],
            &shadow_map,
            texture);
    }

    fb_write_ppm(&fb, "./output.ppm");

    texture_destroy(&texture);
    shadow_map_destroy(&shadow_map);
    free(cube);
}

int main(void) {
    render_single_cube();
    return 0;
}