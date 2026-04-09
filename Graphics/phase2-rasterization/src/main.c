#include "./raster/raster.h"
#include "./geometry/geometry.h"
#include "./framebuffer/framebuffer.h"
#include "../../phase1-math/src/mat/matrix.h"
#include "../../phase1-math/src/camera/camera.h"
#include "../../phase1-math/src/projection/projection.h"
#include "../../phase1-math/src/vec/vector.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define XROT 2.5f
#define YROT 2.2f
#define ZROT 1.2f
#define XSCALE 2.0f
#define YSCALE 2.0f
#define ZSCALE 2.0f
#define TX 0.0f
#define TY 0.0f
#define TZ 0.0f
#define RTEST 200
#define GTEST 200
#define BTEST 255

static void test_full_pipeline(void){
    printf("\n[SUITE] Pipeline\n");

    

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

    struct Matrix scale = scale_constructor(XSCALE, YSCALE, ZSCALE);
    struct Matrix rotate_x = rotation_x_constructor(XROT);
    struct Matrix rotate_y = rotation_y_constructor(YROT);
    struct Matrix rotate_z = rotation_z_constructor(ZROT);
    struct Matrix rotate = mat4_mul(rotate_x, mat4_mul(rotate_y, rotate_z));
    struct Matrix translate = translation_constructor(TX, TY, TZ);
    struct Matrix model = mat4_mul(translate, (mat4_mul(rotate, scale)));

    // define camera position and target
    struct Vector3 camera = {-5.0f, 0.0f ,0.0f};
    struct Vector3 target = {5.0f, 0.0f, 0.0f};
    struct Vector3 up = {0.0f, 1.0f, 0.0f};

    struct Matrix view = lookAt(camera, target, up); // translate and transform into camera space

    // define frustrum values
    float fov = 1.5708f;
    float aspect = 800.0f / 600.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    struct Matrix proj = perspective(fov, aspect, near, far); // project and transform into the clip space
    struct Matrix mv = mat4_mul(view, model);
    struct Matrix mvp = mat4_mul(proj, mv);

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

    for (int i = 0; i < 8; i++) {
        printf("vertex %d: (%.1f, %.1f)\n", i, cube_v3[i].x, cube_v3[i].y);
    }

    // Create a pixel buffer — 800x600, 3 channels (RGB)
    unsigned char image[600][800][3];

    // Initialize to black
    memset(image, 0, sizeof(image));

    for (int i = 0; i < 8; i++){
        // Draw a white pixel at a screen coordinate
        int px = (int)cube_v3[i].x;
        int py = (int)cube_v3[i].y;
        if (px >= 0 && px < 800 && py >= 0 && py < 600) {
            image[py][px][0] = RTEST;  // R
            image[py][px][1] = GTEST;  // G
            image[py][px][2] = BTEST;  // B
        }
    }


    // Write the PPM file
    FILE *f = fopen("output.ppm", "wb");
    fprintf(f, "P6\n800 600\n255\n");
    fwrite(image, 1, sizeof(image), f);
    fclose(f);
}

static void cube_wireframe () {
    
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

    int edges[12][2] = {
        // back face (z = -1): vertices 0,1,2,3
        {0, 1},  // bottom edge
        {1, 2},  // right edge
        {2, 3},  // top edge
        {3, 0},  // left edge

        // front face (z = +1): vertices 4,5,6,7
        {4, 5},  // bottom edge
        {5, 6},  // right edge
        {6, 7},  // top edge
        {7, 4},  // left edge

        // connecting edges (front to back)
        {0, 4},  // bottom left
        {1, 5},  // bottom right
        {2, 6},  // top right
        {3, 7},  // top left
    };

    struct Matrix scale = scale_constructor(XSCALE, YSCALE, ZSCALE);
    struct Matrix rotate_x = rotation_x_constructor(XROT);
    struct Matrix rotate_y = rotation_y_constructor(YROT);
    struct Matrix rotate_z = rotation_z_constructor(ZROT);
    struct Matrix rotate = mat4_mul(rotate_x, mat4_mul(rotate_y, rotate_z));
    struct Matrix translate = translation_constructor(TX, TY, TZ);
    struct Matrix model = mat4_mul(translate, (mat4_mul(rotate, scale)));

    // define camera position and target
    struct Vector3 camera = {-5.0f, 0.0f ,0.0f};
    struct Vector3 target = {5.0f, 0.0f, 0.0f};
    struct Vector3 up = {0.0f, 1.0f, 0.0f};

    struct Matrix view = lookAt(camera, target, up); // translate and transform into camera space

    // define frustrum values
    float fov = 1.5708f;
    float aspect = 800.0f / 600.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    struct Matrix proj = perspective(fov, aspect, near, far); // project and transform into the clip space
    struct Matrix mv = mat4_mul(view, model);
    struct Matrix mvp = mat4_mul(proj, mv);

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

    for (int i = 0; i < 8; i++) {
        printf("vertex %d: (%.1f, %.1f)\n", i, cube_v3[i].x, cube_v3[i].y);
    }
    
    struct Framebuffer fb = fb_create(800,600); // default to 800x600 canvas
    

    for (int i = 0; i < 12; i++){
        int x0 = cube_v3[edges[i][0]].x;
        int y0 = cube_v3[edges[i][0]].y;
        int x1 = cube_v3[edges[i][1]].x;
        int y1 = cube_v3[edges[i][1]].y;
        draw_line(&fb, x0, y0, x1, y1, RTEST, GTEST, BTEST);
    }
    printf("[FRAMEBUFFER] Updating Output File...\n");
    fb_write_ppm(&fb, "./output.ppm"); 
    
}

static void cube_triangles () {
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

    struct Matrix scale = scale_constructor(XSCALE, YSCALE, ZSCALE);
    struct Matrix rotate_x = rotation_x_constructor(XROT);
    struct Matrix rotate_y = rotation_y_constructor(YROT);
    struct Matrix rotate_z = rotation_z_constructor(ZROT);
    struct Matrix rotate = mat4_mul(rotate_x, mat4_mul(rotate_y, rotate_z));
    struct Matrix translate = translation_constructor(TX, TY, TZ);
    struct Matrix model = mat4_mul(translate, (mat4_mul(rotate, scale)));

    // define camera position and target
    struct Vector3 camera = {-5.0f, 0.0f ,0.0f};
    struct Vector3 target = {5.0f, 0.0f, 0.0f};
    struct Vector3 up = {0.0f, 1.0f, 0.0f};

    struct Matrix view = lookAt(camera, target, up); // translate and transform into camera space

    // define frustrum values
    float fov = 1.5708f;
    float aspect = 800.0f / 600.0f;
    float near = 0.1f;
    float far = 100.0f;
    
    struct Matrix proj = perspective(fov, aspect, near, far); // project and transform into the clip space
    struct Matrix mv = mat4_mul(view, model);
    struct Matrix mvp = mat4_mul(proj, mv);

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

    for (int i = 0; i < 8; i++) {
        printf("vertex %d: (%.1f, %.1f)\n", i, cube_v3[i].x, cube_v3[i].y);
    }
    
    struct Framebuffer fb = fb_create(800,600); // default to 800x600 canvas
    
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
        printf("[TEST] Drawing Triangle.\n");
        draw_triangle(&fb,
            triangles[i].v[0],
            triangles[i].v[1],
            triangles[i].v[2],
            (triangles[i].r - (i * 12) % 256),
            (triangles[i].g - (i * 12)) % 256,
            (triangles[i].b - (i * 12))  % 256 );
    }

    printf("[FRAMEBUFFER] Updating Output File...\n");
    fb_write_ppm(&fb, "./output.ppm");
}

int main (void) {
    printf("[TEST] Starting Phase 2 Tests...\n");
    
    test_full_pipeline();
    sleep(1);
    cube_wireframe();
    sleep(1);
    cube_triangles();

    return 0;
}