/*
 * main.c -- test suite for graphics handling
 */

#include "./mat/matrix.h"
#include "./camera/camera.h"
#include "./projection/projection.h"
#include "./vec/vector.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


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
     struct Matrix mvp = mat4_mul(proj, view);

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

int main(void) {
     printf("Phase 1 Math Test Suite\n");
     printf("=======================\n");

     printf("\nRunning full pipeline...\n");
     test_full_pipeline();

     return (g_failed_checks == 0) ? 0 : 1;
}