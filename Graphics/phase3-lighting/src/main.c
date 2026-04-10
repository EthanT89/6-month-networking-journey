#define _POSIX_C_SOURCE 199309L

#include "../../phase1-math/src/mat/matrix.h"
#include "../../phase1-math/src/camera/camera.h"
#include "../../phase1-math/src/projection/projection.h"
#include "../../phase1-math/src/vec/vector.h"
#include "../../phase2-rasterization/src/raster/raster.h"
#include "../../phase2-rasterization/src/framebuffer/framebuffer.h"
#include "./geometry/geometry.h"
#include "./lighting/lighting.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define XROT 2.5f
#define YROT 2.3f
#define ZROT 1.0f
#define XSCALE 1.0f
#define YSCALE 1.0f
#define ZSCALE 1.0f
#define TX 0.0f
#define TY 0.0f
#define TZ 0.0f
#define RTEST 255
#define GTEST 150
#define BTEST 255
#define AMBIENT 0.1f


int cube_triangles_normals (float lx, float ly, float lz) {
    static struct Framebuffer fb;
    static bool framebuffer_initialized = false;

    // simple cube vertices
    struct Vector4 cube[8] = {
        {-1, -1, -1, 1}, 
        { 1, -1, -1, 1},
        { 1,  1, -1, 1}, 
        {-1,  1, -1, 1}, 
        {-1, -1,  1, 1}, 
        { 1, -1,  1, 1},
        { 1,  1,  1, 1}, 
        {-1,  1,  1, 1}, 
    };

    int tri_indices[12][3] = {
        // front face (z=+1)
        {4, 5, 6}, {4, 6, 7},
        // back face (z=-1)
        {1, 0, 3}, {1, 3, 2},
        // right face (x=+1)
        {5, 1, 2}, {5, 2, 6},
        // left face (x=-1)
        {0, 4, 7}, {0, 7, 3},
        // top face (y=+1)
        {7, 6, 2}, {7, 2, 3},
        // bottom face (y=-1)
        {0, 1, 5}, {0, 5, 4},
    };

    struct Matrix scale = scale_constructor(XSCALE, YSCALE, ZSCALE);
    struct Matrix rotate_x = rotation_x_constructor(XROT);
    struct Matrix rotate_y = rotation_y_constructor(YROT);
    struct Matrix rotate_z = rotation_z_constructor(ZROT);
    struct Matrix rotate = mat4_mul(rotate_x, mat4_mul(rotate_y, rotate_z));
    struct Matrix translate = translation_constructor(TX, TY, TZ);
    struct Matrix model = mat4_mul(translate, (mat4_mul(rotate, scale)));

    // define lighting position
    struct DirectionalLight light;
    light.direction = v3_norm((struct Vector3){lx, ly, lz});
    light.intensity = 1.0f;

    // define camera position and target
    struct Vector3 camera = {-5.0f, 0.0f ,0.0f};
    struct Vector3 target = {5.0f, 0.0f, 0.0f};
    struct Vector3 up = {0.0f, 1.0f, 0.0f};
     
    // BRANCH 1 — world space (for lighting)
    struct Vector3 world_verts[8]; // Captured world space coordinates for triangle normal calculations
    for (int i = 0; i < 8; i++){
        struct Vector4 w = mat4_mul_vec4(model, cube[i]);
        world_verts[i].x = w.x;
        world_verts[i].y = w.y;
        world_verts[i].z = w.z;
    }

    // get triangle normals
    struct Triangle world_triangles[12];
    for (int i = 0; i < 12; i++){
        world_triangles[i].v[0] = world_verts[tri_indices[i][0]];
        world_triangles[i].v[1] = world_verts[tri_indices[i][1]];
        world_triangles[i].v[2] = world_verts[tri_indices[i][2]];
        world_triangles[i].r = RTEST;
        world_triangles[i].g = GTEST;
        world_triangles[i].b = BTEST;
        world_triangles[i].normal = compute_face_normal(world_triangles[i].v[0], world_triangles[i].v[1], world_triangles[i].v[2]);
    }

    struct Vector3 vert_norms[8];
    for (int i = 0; i < 8; i++){
        struct Vector3 sum = {0, 0, 0};
        for (int j = 0; j < 12; j++){
            if ( tri_indices[j][0] == i || tri_indices[j][1] == i || tri_indices[j][2] == i){
                sum = v3_addition(sum, world_triangles[j].normal);
            }
        }
        vert_norms[i] = v3_norm(sum);
    }

    // compute lighting
    for (int i = 0; i < 12; i++){

        for (int j = 0; j < 3; j++){
            float diffuse = compute_diffuse(vert_norms[tri_indices[i][j]], light);
            float specular = 0.0f;
            if (diffuse > 0.0f){
                struct Vector3 midpoint = get_triangle_midpoint(&world_triangles[i]);
                struct Vector3 view_dir = v3_norm(v3_subtraction(camera, midpoint));
                specular = compute_specular(world_triangles[i].normal, light.direction, view_dir, 16.0f); 
            }

            world_triangles[i].brightness[j] = fmin((AMBIENT + diffuse + specular), 1.0f);
            // printf("brightness of %d: %f\n", i, world_triangles[i].brightness[j]);
        }
    }

    struct Matrix view = lookAt(camera, target, up); // translate and transform into camera space

    // define frustrum values
    float fov = 1.5708f;
    float aspect = 800.0f / 600.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    struct Matrix proj = perspective(fov, aspect, near, far); // project and transform into the clip space
    struct Matrix mvp = mat4_mul(proj, mat4_mul(view, model));

    for (int i = 0; i < 8; i++){
        cube[i] = mat4_mul_vec4(mvp, cube[i]); // project onto clip space
    }

    struct Vector3 cube_v3[8];
    for (int i = 0; i < 8; i++){
        cube_v3[i] = perspective_divide(cube[i]); // maps to NDC
    }
    for (int i = 0; i < 8; i++){
        cube_v3[i] = viewport(cube_v3[i], 600.0f, 800.0f); // maps to Viewport space
    }
    
    if (!framebuffer_initialized) {
        fb = fb_create(800, 600); // default to 800x600 canvas
        framebuffer_initialized = true;
    }
    fb_clear(&fb);

    struct Triangle triangles[12];
    for (int i = 0; i < 12; i++){
        triangles[i].v[0] = cube_v3[tri_indices[i][0]];
        triangles[i].v[1] = cube_v3[tri_indices[i][1]];
        triangles[i].v[2] = cube_v3[tri_indices[i][2]];
        triangles[i].r = RTEST;
        triangles[i].g = GTEST;
        triangles[i].b = BTEST;
    }

    for (int i = 0; i < 12; i++){
        if (is_back_face(&triangles[i])) continue;
        // printf("[TEST] Drawing Triangle.\n");
        draw_triangle(&fb,
            triangles[i].v[0],
            triangles[i].v[1],
            triangles[i].v[2],
            world_triangles[i].brightness[0],
            world_triangles[i].brightness[1],
            world_triangles[i].brightness[2],
            world_triangles[i].r,
            world_triangles[i].g,
            world_triangles[i].b);
    }

    fb_write_ppm(&fb, "./output.ppm");
    return 1;
}

int main (void) {
    struct timespec req = { .tv_sec = 0, .tv_nsec = 16 * 1000 * 1000 };
    float lx = 1.0f;
    float ly = -1.0f;
    float lz = -1.0f;

    float two_pi = 3.14159265358979323846f * 2.0f;
    float angle = 0.0f;
    
    while (cube_triangles_normals(lx, ly, lz) == 1){
        lz = sinf(angle);
        ly = cosf(angle);
        angle = fmodf(angle + 0.01f, two_pi);
        nanosleep(&req, NULL);
    }

    return 0;
}