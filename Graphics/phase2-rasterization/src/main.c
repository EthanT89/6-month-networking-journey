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

static void initial_test () {
    printf("[FRAMEBUFFER] Tests Running...\n");
    printf("[FRAMEBUFFER] Initializing Blank Framebuffer\n");

    struct Framebuffer fb = fb_create(800,600); // default to 800x600 canvas
    printf("[FRAMEBUFFER] Updating Output File...\n");
    fb_write_ppm(&fb, "./output.ppm"); 
    sleep(1);

    printf("[FRAMEBUFFER] Cur Depth Values (random):\n");
    printf("    %f, %f, %f, %f\n", fb.depth[4], fb.depth[400], fb.depth[68], fb.depth[900]);

    if (fb_depth_test(&fb, 50,50,50) == 1){
        fb_set_pixel(&fb, 50,50, 200,100,150);
        printf("[FRAMEBUFFER] Updating Output File...\n");
        fb_write_ppm(&fb, "./output.ppm"); 
        sleep(1);
    }

    if (fb_depth_test(&fb, 51,50,50) == 1){
        fb_set_pixel(&fb, 51,50, 200,100,150);
        printf("[FRAMEBUFFER] Updating Output File...\n");
        fb_write_ppm(&fb, "./output.ppm"); 
        sleep(1);
    }

    if (fb_depth_test(&fb, 50,51,50) == 1){
        fb_set_pixel(&fb, 50,51, 200,100,150);
        printf("[FRAMEBUFFER] Updating Output File...\n");
        fb_write_ppm(&fb, "./output.ppm"); 
        sleep(1);
    }

    if (fb_depth_test(&fb, 51,51,50) == 1){
        fb_set_pixel(&fb, 51,51, 200,100,150);
        printf("[FRAMEBUFFER] Updating Output File...\n");
        fb_write_ppm(&fb, "./output.ppm"); 
        sleep(1);
    }

    fb_clear(&fb);
    printf("[FRAMEBUFFER] Updating Output File...\n");
    fb_write_ppm(&fb, "./output.ppm"); 
    sleep(1);

    fb_destroy(&fb);
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

    struct Matrix scale = scale_constructor(1.0f, 2.0f, 0.5f);
    struct Matrix rotate_x = rotation_x_constructor(2.0f);
    struct Matrix rotate_y = rotation_y_constructor(0.5f);
    struct Matrix rotate_z = rotation_z_constructor(2.0f);
    struct Matrix rotate = mat4_mul(rotate_x, mat4_mul(rotate_y, rotate_z));
    struct Matrix translate = translation_constructor(0.0f, 0.0f, 0.0f);
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
        draw_line(&fb, x0, y0, x1, y1, 255, 255, 255);
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

    struct Matrix scale = scale_constructor(1.0f, 1.0f, 1.0f);
    struct Matrix rotate_x = rotation_x_constructor(5.0f);
    struct Matrix rotate_y = rotation_y_constructor(4.3f);
    struct Matrix rotate_z = rotation_z_constructor(3.0f);
    struct Matrix rotate = mat4_mul(rotate_x, mat4_mul(rotate_y, rotate_z));
    struct Matrix translate = translation_constructor(0.0f, 0.0f, 0.0f);
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
        triangles[i].r = 255;
        triangles[i].g = 255;
        triangles[i].b = 255;
    }

    for (int i = 0; i < 12; i++){
        if (is_back_face(&triangles[i])) continue;
        printf("[TEST] Drawing Triangle.\n");
        draw_triangle(&fb,
            triangles[i].v[0],
            triangles[i].v[1],
            triangles[i].v[2],
            triangles[i].r - (i * 12),
            triangles[i].g - (i * 12),
            triangles[i].b - (i * 12));
    }

    printf("[FRAMEBUFFER] Updating Output File...\n");
    fb_write_ppm(&fb, "./output.ppm");
}

int main (void) {
    printf("[TEST] Starting Phase 2 Tests...\n");
    
    //initial_test();
    //cube_wireframe();
    cube_triangles();

    return 0;
}