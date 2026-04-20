/*
 * main.c -- testing file for phase 4 texture and shadow integration
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

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#define FB_WIDTH 800
#define FB_HEIGHT 600
#define CUBE_VERTEX_COUNT 8
#define CUBE_TRI_COUNT 12

#define RTEST 255
#define GTEST 150
#define BTEST 255
#define AMBIENT 0.1f

static const int cube_tri_indices[CUBE_TRI_COUNT][3] = {
    {4, 5, 6}, {4, 6, 7},
    {1, 0, 3}, {1, 3, 2},
    {5, 1, 2}, {5, 2, 6},
    {0, 4, 7}, {0, 7, 3},
    {7, 6, 2}, {7, 2, 3},
    {0, 1, 5}, {0, 5, 4},
};

static const float cube_uvs[CUBE_TRI_COUNT][3][2] = {
    // front face {4,5,6}
    {{0,1}, {1,1}, {1,0}},
    // front face {4,6,7}D
    {{0,1}, {1,0}, {0,0}}, 
    // back face {1, 0, 3}
    {{0,1}, {1,1}, {1,0}},
    // back face {1, 3, 2}
    {{0,1}, {1,0}, {0,0}}, 
    // right side face {5, 1, 2}
    {{0,1}, {1,1}, {1,0}},
    // right side face {5, 2, 6}
    {{0,1}, {1,0}, {0,0}}, 
    // left side face {0, 4, 7}
    {{0,1}, {1,1}, {1,0}},
    // left side face {0, 7, 3}
    {{0,1}, {1,0}, {0,0}}, 
    // top face {7, 6, 2}
    {{0,1}, {1,1}, {1,0}},
    // top face {7, 2, 3}
    {{0,1}, {1,0}, {0,0}}, 
    // bottom face {0, 1, 5}
    {{0,1}, {1,1}, {1,0}},
    // bottom face {0, 5, 4}
    {{0,1}, {1,0}, {0,0}}, 
};

static struct Vector4 *create_cube_vertices(void) {
    struct Vector4 *cube = malloc(CUBE_VERTEX_COUNT * sizeof(struct Vector4));
    if (!cube) return NULL;
    cube[0] = (struct Vector4){-1, -1, -1, 1};
    cube[1] = (struct Vector4){ 1, -1, -1, 1};
    cube[2] = (struct Vector4){ 1,  1, -1, 1};
    cube[3] = (struct Vector4){-1,  1, -1, 1};
    cube[4] = (struct Vector4){-1, -1,  1, 1};
    cube[5] = (struct Vector4){ 1, -1,  1, 1};
    cube[6] = (struct Vector4){ 1,  1,  1, 1};
    cube[7] = (struct Vector4){-1,  1,  1, 1};

    return cube;
}

static void build_model_space_data(
    const struct Vector4 cube[CUBE_VERTEX_COUNT],
    struct Matrix model,
    struct Vector3 world_verts[CUBE_VERTEX_COUNT],
    struct Triangle world_triangles[CUBE_TRI_COUNT],
    struct Vector3 vert_norms[CUBE_VERTEX_COUNT])
{
    for (int vi = 0; vi < CUBE_VERTEX_COUNT; vi++){
        struct Vector4 w = mat4_mul_vec4(model, cube[vi]);
        world_verts[vi] = (struct Vector3){w.x, w.y, w.z};
    }

    for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
        world_triangles[ti].v_cam[0] = world_verts[cube_tri_indices[ti][0]];
        world_triangles[ti].v_cam[1] = world_verts[cube_tri_indices[ti][1]];
        world_triangles[ti].v_cam[2] = world_verts[cube_tri_indices[ti][2]];
        world_triangles[ti].r = RTEST;
        world_triangles[ti].g = GTEST;
        world_triangles[ti].b = BTEST;
        world_triangles[ti].normal = compute_face_normal(
            world_triangles[ti].v_cam[0],
            world_triangles[ti].v_cam[1],
            world_triangles[ti].v_cam[2]);
    }

    for (int vi = 0; vi < CUBE_VERTEX_COUNT; vi++){
        struct Vector3 sum = {0, 0, 0};
        for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
            if (cube_tri_indices[ti][0] == vi || cube_tri_indices[ti][1] == vi || cube_tri_indices[ti][2] == vi){
                sum = v3_addition(sum, world_triangles[ti].normal);
            }
        }
        vert_norms[vi] = v3_norm(sum);
    }
}

static void compute_triangle_lighting(
    struct Triangle world_triangles[CUBE_TRI_COUNT],
    const struct Vector3 vert_norms[CUBE_VERTEX_COUNT],
    struct DirectionalLight light,
    struct Vector3 camera)
{
    for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
        for (int j = 0; j < 3; j++){
            int vertex_id = cube_tri_indices[ti][j];
            float diffuse = compute_diffuse(vert_norms[vertex_id], light);
            float specular = 0.0f;

            if (diffuse > 0.0f){
                struct Vector3 midpoint = get_triangle_midpoint(&world_triangles[ti]);
                struct Vector3 view_dir = v3_norm(v3_subtraction(camera, midpoint));
                specular = compute_specular(world_triangles[ti].normal, light.direction, view_dir, 16.0f);
            }

            world_triangles[ti].brightness[j] = fminf(AMBIENT + diffuse + specular, 1.0f);
        }
    }
}

static void render_ground_plane(struct Framebuffer *fb){

}

static void render_single_cube(void) {
    static struct Framebuffer fb;
    static struct Texture texture;
    static bool framebuffer_initialized = false;

    texture_load((const char *)"./content/mountains-lake.jpg", &texture);

    struct Vector4 *cube = create_cube_vertices();
    if (!cube) return;

    float translation[3] = {0.0f, 0.0f, 2.0f};
    float rotation[3] = {3.0f, 2.1f, 2.2f};
    float scale_value = 2.0f;

    // define lighting position
    struct DirectionalLight light;
    light.direction = v3_norm((struct Vector3){-1, -1, 1});
    light.intensity = 1.0f;

    // define camera position and target
    struct Vector3 camera = {-5.0f, 0.0f ,0.0f};
    struct Vector3 target = {5.0f, 0.0f, 0.0f};
    struct Vector3 up = {0.0f, 1.0f, 0.0f};

    struct Vector3 world_verts[CUBE_VERTEX_COUNT];
    struct Triangle world_triangles[CUBE_TRI_COUNT];
    struct Vector3 vert_norms[CUBE_VERTEX_COUNT];
    struct Vector4 clip_verts[CUBE_VERTEX_COUNT];

    if (!framebuffer_initialized) {
        fb = fb_create(FB_WIDTH, FB_HEIGHT);
        framebuffer_initialized = true;
    }
    fb_clear(&fb);

    struct Matrix translate = translation_constructor(translation[0], translation[1], translation[2]);
    struct Matrix rotx = rotation_x_constructor(rotation[0]);
    struct Matrix roty = rotation_y_constructor(rotation[1]);
    struct Matrix rotz = rotation_z_constructor(rotation[2]);
    struct Matrix scale = scale_constructor(scale_value, scale_value, scale_value);

    struct Matrix rotate = mat4_mul(rotx, mat4_mul(roty, rotz));
    struct Matrix model = mat4_mul(translate, mat4_mul(rotate, scale));

    build_model_space_data(cube, model, world_verts, world_triangles, vert_norms);
    compute_triangle_lighting(world_triangles, vert_norms, light, camera);

    struct Matrix view = lookAt(camera, target, up); // translate and transform into camera space

    // define frustrum values
    float fov = 1.5708f;
    float aspect = (float)FB_WIDTH / (float)FB_HEIGHT;
    float near = 0.1f;
    float far = 100.0f;
    
    struct Matrix proj = perspective(fov, aspect, near, far); // project and transform into the clip space
    struct Matrix mvp = mat4_mul(proj, mat4_mul(view, model));

    for (int j = 0; j < CUBE_VERTEX_COUNT; j++){
        clip_verts[j] = mat4_mul_vec4(mvp, cube[j]); // project onto clip space
    }

    struct Vector3 cube_v3[CUBE_VERTEX_COUNT];
    for (int j = 0; j < CUBE_VERTEX_COUNT; j++){
        struct Vector4 ndc = perspective_divide(clip_verts[j]); // maps to NDC
        struct Vector4 viewport_v = viewport(ndc, FB_HEIGHT, FB_WIDTH); // maps to Viewport space
        cube_v3[j] = (struct Vector3){viewport_v.x, viewport_v.y, viewport_v.z};
    }

    struct Triangle triangles[CUBE_TRI_COUNT];
    for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
        triangles[ti].v_cam[0] = cube_v3[cube_tri_indices[ti][0]];
        triangles[ti].v_cam[1] = cube_v3[cube_tri_indices[ti][1]];
        triangles[ti].v_cam[2] = cube_v3[cube_tri_indices[ti][2]];
        triangles[ti].uv[0] = *(struct Vector2*)cube_uvs[ti][0];
        triangles[ti].uv[1] = *(struct Vector2*)cube_uvs[ti][1];
        triangles[ti].uv[2] = *(struct Vector2*)cube_uvs[ti][2];
        triangles[ti].clip_w[0] = clip_verts[cube_tri_indices[ti][0]].w;
        triangles[ti].clip_w[1] = clip_verts[cube_tri_indices[ti][1]].w;
        triangles[ti].clip_w[2] = clip_verts[cube_tri_indices[ti][2]].w;
        triangles[ti].r = RTEST;
        triangles[ti].g = GTEST;
        triangles[ti].b = BTEST;
    }

    for (int ti = 0; ti < CUBE_TRI_COUNT; ti++){
        if (is_back_face(&triangles[ti])) continue;
        draw_triangle_textured(&fb,
            triangles[ti].v_cam[0],
            triangles[ti].v_cam[1],
            triangles[ti].v_cam[2],
            triangles[ti].uv[0],
            triangles[ti].uv[1],
            triangles[ti].uv[2],
            world_triangles[ti].brightness[0],
            world_triangles[ti].brightness[1],
            world_triangles[ti].brightness[2],
            triangles[ti].clip_w[0],
            triangles[ti].clip_w[1],
            triangles[ti].clip_w[2],
            texture);
    }

    fb_write_ppm(&fb, "./output.ppm");
    free(cube);
}

int main (void) {
    render_single_cube();

    return 0;
}