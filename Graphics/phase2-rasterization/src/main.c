#include "./raster/raster.h"
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

     struct Matrix scale = scale_constructor(1.0f, 1.0f, 1.0f);
     struct Matrix rotate_x = rotation_x_constructor(3.0f);
     struct Matrix rotate_y = rotation_y_constructor(3.5f);
     struct Matrix rotate_z = rotation_z_constructor(0.5f);
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

     // Create a pixel buffer — 800x600, 3 channels (RGB)
     unsigned char image[600][800][3];

     // Initialize to black
     memset(image, 0, sizeof(image));

     for (int i = 0; i < 8; i++){
          // Draw a white pixel at a screen coordinate
          int px = (int)cube_v3[i].x;
          int py = (int)cube_v3[i].y;
          if (px >= 0 && px < 800 && py >= 0 && py < 600) {
               image[py][px][0] = 255;  // R
               image[py][px][1] = 255;  // G
               image[py][px][2] = 255;  // B
          }
     }


     // Write the PPM file
     FILE *f = fopen("output.ppm", "wb");
     fprintf(f, "P6\n800 600\n255\n");
     fwrite(image, 1, sizeof(image), f);
     fclose(f);
}

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

    struct Matrix scale = scale_constructor(1.0f, 1.0f, 1.0f);
    struct Matrix rotate_x = rotation_x_constructor(3.0f);
    struct Matrix rotate_y = rotation_y_constructor(3.5f);
    struct Matrix rotate_z = rotation_z_constructor(0.5f);
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
    

    // After the full projection pipeline, we now have the correct coordinates of the cube. We now need edges.

    struct Framebuffer fb = fb_create(800,600); // default to 800x600 canvas

    for (int i = 0; i < 12; i++){
        int x0 = cube_v3[edges[i][0]].x;
        int y0 = cube_v3[edges[i][0]].y;
        int x1 = cube_v3[edges[i][1]].x;
        int y1 = cube_v3[edges[i][1]].y;
        draw_line(&fb, x0, y0, x1, y1, 100, 100, 100);
    }
    printf("[FRAMEBUFFER] Updating Output File...\n");
    fb_write_ppm(&fb, "./output.ppm"); 
    
}

int main (void) {
    printf("[TEST] Starting Phase 2 Tests...\n");
    
    initial_test();

    return 0;
}