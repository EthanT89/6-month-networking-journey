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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define OBJ_COUNT 1
#define RTEST 255
#define GTEST 150
#define BTEST 255
#define AMBIENT 0.1f

struct Vector4 * generate_cube() {
    struct Vector4 *cube = malloc((8 * sizeof (struct Vector4)));
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

static void tests () {
    static struct Framebuffer fb;
    static bool framebuffer_initialized = false;

    struct Vector4 *cube1 = generate_cube();
    struct Vector4 *cube2 = generate_cube();
    struct Vector4 *cube3 = generate_cube();

    int tri_indices_cube[12][3] = {
        {4, 5, 6}, {4, 6, 7}, // front face (z=+1)
        {1, 0, 3}, {1, 3, 2}, // back face (z=-1)
        {5, 1, 2}, {5, 2, 6}, // right face (x=+1)
        {0, 4, 7}, {0, 7, 3}, // left face (x=-1)
        {7, 6, 2}, {7, 2, 3}, // top face (y=+1)
        {0, 1, 5}, {0, 5, 4}, // bottom face (y=-1)
    };

    struct Vector4 **cubes = {
        &cube1,
        &cube2,
        &cube3,
    };

    float translations[3][3] = {
        {0.0f, 0.0f, 0.0f},
        {5.0f, 0.0f, 0.0f},
        (0.0f, 5.0f, 0.0f)
    };

    float rotations[3][3] = {
        {3.0f, 0.0f, 2.2f},
        {5.0f, 4.0f, 0.5f},
        (0.0f, 5.0f, 1.0f)
    };

    float scales[3] = {1.0f, 2.0f, 1.5f};

    // define lighting position
    struct DirectionalLight light;
    light.direction = v3_norm((struct Vector3){2, 5, 2});
    light.intensity = 1.0f;

    // define camera position and target
    struct Vector3 camera = {-5.0f, 0.0f ,0.0f};
    struct Vector3 target = {5.0f, 0.0f, 0.0f};
    struct Vector3 up = {0.0f, 1.0f, 0.0f};

    struct Vector4 world_verts[3][8]; // Captured world space coordinates for triangle normal calculations
    struct Triangle world_triangles[3][12];
    struct Vector3 vert_norms[3][8];

    if (!framebuffer_initialized) {
        fb = fb_create(800, 600); // default to 800x600 canvas
        framebuffer_initialized = true;
    }
    fb_clear(&fb);

    for (int i = 0; i < 3; i++){
        struct Matrix translate = translation_constructor(translations[i][0], translations[i][1], translations[i][2]);
        struct Matrix rotx = rotation_x_constructor(rotations[i][0]);
        struct Matrix roty = rotation_y_constructor(rotations[i][1]);
        struct Matrix rotz = rotation_z_constructor(rotations[i][2]);
        struct Matrix scale = scale_constructor(scales[i], scales[i], scales[i]);

        struct Matrix rotate = mat4_mul(rotx, mat4_mul(roty, rotz));
        struct Matrix model = mat4_mul(translate, mat4_mul(rotate, scale));

        // BRANCH 1 — world space (for lighting)
        for (int j = 0; j < 8; j++){
            struct Vector4 w = mat4_mul_vec4(model, cubes[i][j]);
            world_verts[i][j].x = w.x;
            world_verts[i][j].y = w.y;
            world_verts[i][j].z = w.z;
        }

        for (int j = 0; j < 12; j++){
            world_triangles[i][j].v[0] = *world_verts[tri_indices_cube[j][0]];
            world_triangles[i][j].v[1] = *world_verts[tri_indices_cube[j][1]];
            world_triangles[i][j].v[2] = *world_verts[tri_indices_cube[j][2]];
            world_triangles[i][j].r = RTEST;
            world_triangles[i][j].g = GTEST;
            world_triangles[i][j].b = BTEST;
            world_triangles[i][j].normal = compute_face_normal(world_triangles[i][j].v[0], world_triangles[i][j].v[1], world_triangles[i][j].v[2]);
        }

        for (int k = 0; k < 8; k++){
            struct Vector3 sum = {0, 0, 0};
            for (int j = 0; j < 12; j++){
                if ( tri_indices_cube[j][0] == k || tri_indices_cube[j][1] == k || tri_indices_cube[j][2] == k){
                    sum = v3_addition(sum, world_triangles[i][j].normal);
                }
            }
            vert_norms[i][k] = v3_norm(sum);
        }


        // compute lighting
        for (int k = 0; k < 12; k++){

            for (int j = 0; j < 3; j++){
                float diffuse = compute_diffuse(*vert_norms[tri_indices_cube[k][j]], light);
                float specular = 0.0f;
                if (diffuse > 0.0f){
                    struct Vector3 midpoint = get_triangle_midpoint(&world_triangles[i][k]);
                    struct Vector3 view_dir = v3_norm(v3_subtraction(camera, midpoint));
                    specular = compute_specular(world_triangles[i][k].normal, light.direction, view_dir, 16.0f); 
                }

                world_triangles[i][k].brightness[j] = fmin((AMBIENT + diffuse + specular), 1.0f);
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

        for (int j = 0; j < 8; j++){
            cubes[i][j] = mat4_mul_vec4(mvp, cubes[i][j]); // project onto clip space
        }

        struct Vector4 cube_v3[8];
        for (int j = 0; j < 8; j++){
            cube_v3[j] = perspective_divide(cubes[i][j]); // maps to NDC
        }
        for (int j = 0; j < 8; j++){
            cube_v3[j] = viewport(cube_v3[j], 600.0f, 800.0f); // maps to Viewport space
        }

        struct Triangle triangles[12];
        for (int j = 0; j < 12; j++){
            triangles[j].v[0] = cube_v3[tri_indices_cube[j][0]];
            triangles[j].v[1] = cube_v3[tri_indices_cube[j][1]];
            triangles[j].v[2] = cube_v3[tri_indices_cube[j][2]];
            triangles[j].r = RTEST;
            triangles[j].g = GTEST;
            triangles[j].b = BTEST;
        }

        for (int j = 0; j < 12; j++){
            if (is_back_face(&triangles[j])) continue;
            // printf("[TEST] Drawing Triangle.\n");
            draw_triangle(&fb,
                triangles[j].v[0],
                triangles[j].v[1],
                triangles[j].v[2],
                world_triangles[i][j].brightness[0],
                world_triangles[i][j].brightness[1],
                world_triangles[i][j].brightness[2],
                world_triangles[i][j].r,
                world_triangles[i][j].g,
                world_triangles[i][j].b);
        }
    }

    fb_write_ppm(&fb, "./output.ppm");
    return 1;
}

int main (void) {

    tests();

    return 0;
}