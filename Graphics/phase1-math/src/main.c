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

static const float EPSILON = 1e-5f;
static int g_total_checks = 0;
static int g_failed_checks = 0;

static bool float_close(float a, float b) {
     return fabsf(a - b) <= EPSILON;
}

static void check_float(float actual, float expected, const char *label) {
     g_total_checks++;
     if (float_close(actual, expected)) {
          printf("  [PASS] %s\n", label);
     } else {
          g_failed_checks++;
          printf("  [FAIL] %s (expected %.6f, got %.6f)\n", label, expected, actual);
     }
}

static void check_vec3(struct Vector3 actual, struct Vector3 expected, const char *label) {
     bool ok = float_close(actual.x, expected.x)
          && float_close(actual.y, expected.y)
          && float_close(actual.z, expected.z);

     g_total_checks++;
     if (ok) {
          printf("  [PASS] %s\n", label);
     } else {
          g_failed_checks++;
          printf(
               "  [FAIL] %s (expected {%.6f, %.6f, %.6f}, got {%.6f, %.6f, %.6f})\n",
               label,
               expected.x,
               expected.y,
               expected.z,
               actual.x,
               actual.y,
               actual.z
          );
     }
}

static void check_vec4(struct Vector4 actual, struct Vector4 expected, const char *label) {
     bool ok = float_close(actual.x, expected.x)
          && float_close(actual.y, expected.y)
          && float_close(actual.z, expected.z)
          && float_close(actual.w, expected.w);

     g_total_checks++;
     if (ok) {
          printf("  [PASS] %s\n", label);
     } else {
          g_failed_checks++;
          printf(
               "  [FAIL] %s (expected {%.6f, %.6f, %.6f, %.6f}, got {%.6f, %.6f, %.6f, %.6f})\n",
               label,
               expected.x,
               expected.y,
               expected.z,
               expected.w,
               actual.x,
               actual.y,
               actual.z,
               actual.w
          );
     }
}

static void check_mat4(struct Matrix actual, struct Matrix expected, const char *label) {
     bool ok = true;
     for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
               if (!float_close(actual.m[i][j], expected.m[i][j])) {
                    ok = false;
               }
          }
     }

     g_total_checks++;
     if (ok) {
          printf("  [PASS] %s\n", label);
     } else {
          g_failed_checks++;
          printf("  [FAIL] %s\n", label);
     }
}

static void test_vector3_suite(void) {
     printf("\n[SUITE] Vector3\n");

     struct Vector3 a = {1.0f, -2.0f, 3.5f};
     struct Vector3 b = {-4.0f, 2.0f, 0.5f};

     check_vec3(v3_addition(a, b), (struct Vector3){-3.0f, 0.0f, 4.0f}, "v3_addition");
     check_vec3(v3_subtraction(a, b), (struct Vector3){5.0f, -4.0f, 3.0f}, "v3_subtraction");
     check_vec3(v3_scalar_mult(a, 2.0f), (struct Vector3){2.0f, -4.0f, 7.0f}, "v3_scalar_mult");

     check_float(v3_dot(a, b), -6.25f, "v3_dot");
     check_vec3(
          v3_cross((struct Vector3){1.0f, 0.0f, 0.0f}, (struct Vector3){0.0f, 1.0f, 0.0f}),
          (struct Vector3){0.0f, 0.0f, 1.0f},
          "v3_cross"
     );

     check_float(v3_magnitude((struct Vector3){3.0f, 0.0f, 4.0f}), 5.0f, "v3_magnitude");
     check_vec3(v3_norm((struct Vector3){3.0f, 0.0f, 4.0f}), (struct Vector3){0.6f, 0.0f, 0.8f}, "v3_norm");
     check_vec3(v3_norm((struct Vector3){0.0f, 0.0f, 0.0f}), (struct Vector3){0.0f, 0.0f, 0.0f}, "v3_norm_zero_safe");
}

static void test_vector4_suite(void) {
     printf("\n[SUITE] Vector4\n");

     struct Vector4 a = {1.0f, -2.0f, 3.5f, 2.0f};
     struct Vector4 b = {-4.0f, 2.0f, 0.5f, -1.0f};

     check_vec4(v4_addition(a, b), (struct Vector4){-3.0f, 0.0f, 4.0f, 1.0f}, "v4_addition");
     check_vec4(v4_subtraction(a, b), (struct Vector4){5.0f, -4.0f, 3.0f, 3.0f}, "v4_subtraction");
     check_vec4(v4_scalar_mult(a, 2.0f), (struct Vector4){2.0f, -4.0f, 7.0f, 4.0f}, "v4_scalar_mult");

     check_float(v4_dot(a, b), -8.25f, "v4_dot");
     check_vec4(
          v4_cross((struct Vector4){1.0f, 0.0f, 0.0f, 99.0f}, (struct Vector4){0.0f, 1.0f, 0.0f, -5.0f}),
          (struct Vector4){0.0f, 0.0f, 1.0f, 0.0f},
          "v4_cross"
     );

     check_float(v4_magnitude((struct Vector4){2.0f, 3.0f, 6.0f, 99.0f}), 7.0f, "v4_magnitude");
     check_vec4(v4_norm((struct Vector4){0.0f, 3.0f, 4.0f, 10.0f}), (struct Vector4){0.0f, 0.6f, 0.8f, 10.0f}, "v4_norm");
     check_vec4(v4_norm((struct Vector4){0.0f, 0.0f, 0.0f, 9.0f}), (struct Vector4){0.0f, 0.0f, 0.0f, 9.0f}, "v4_norm_zero_safe");
}

static void test_matrix_suite(void) {
     printf("\n[SUITE] Matrix4x4\n");

     struct Matrix identity = identity_matrix_constructor();
     check_mat4(identity, (struct Matrix){
          .m = {
               {1.0f, 0.0f, 0.0f, 0.0f},
               {0.0f, 1.0f, 0.0f, 0.0f},
               {0.0f, 0.0f, 1.0f, 0.0f},
               {0.0f, 0.0f, 0.0f, 1.0f},
          }
     }, "identity_matrix_constructor");

     struct Matrix a = {
          .m = {
               {1.0f, 2.0f, 3.0f, 4.0f},
               {5.0f, 6.0f, 7.0f, 8.0f},
               {9.0f, 10.0f, 11.0f, 12.0f},
               {13.0f, 14.0f, 15.0f, 16.0f},
          }
     };
     struct Matrix b = {
          .m = {
               {17.0f, 18.0f, 19.0f, 20.0f},
               {21.0f, 22.0f, 23.0f, 24.0f},
               {25.0f, 26.0f, 27.0f, 28.0f},
               {29.0f, 30.0f, 31.0f, 32.0f},
          }
     };

     check_mat4(mat4_mul(identity, a), a, "mat4_mul_identity_left");
     check_mat4(mat4_mul(a, identity), a, "mat4_mul_identity_right");
     check_mat4(mat4_mul(a, b), (struct Matrix){
          .m = {
               {250.0f, 260.0f, 270.0f, 280.0f},
               {618.0f, 644.0f, 670.0f, 696.0f},
               {986.0f, 1028.0f, 1070.0f, 1112.0f},
               {1354.0f, 1412.0f, 1470.0f, 1528.0f},
          }
     }, "mat4_mul_general");

     struct Matrix t = {
          .m = {
               {1.0f, 2.0f, 3.0f, 4.0f},
               {0.0f, 1.0f, 0.0f, 5.0f},
               {6.0f, 7.0f, 8.0f, 9.0f},
               {0.0f, 0.0f, 0.0f, 1.0f},
          }
     };
     check_mat4(transpose(t), (struct Matrix){
          .m = {
               {1.0f, 0.0f, 6.0f, 0.0f},
               {2.0f, 1.0f, 7.0f, 0.0f},
               {3.0f, 0.0f, 8.0f, 0.0f},
               {4.0f, 5.0f, 9.0f, 1.0f},
          }
     }, "transpose");

     struct Matrix translation = translation_constructor(5.0f, -2.0f, 10.0f);
     check_vec4(
          mat4_mul_vec4(translation, (struct Vector4){1.0f, 2.0f, 3.0f, 1.0f}),
          (struct Vector4){6.0f, 0.0f, 13.0f, 1.0f},
          "translation_constructor_point"
     );
     check_vec4(
          mat4_mul_vec4(translation, (struct Vector4){1.0f, 2.0f, 3.0f, 0.0f}),
          (struct Vector4){1.0f, 2.0f, 3.0f, 0.0f},
          "translation_constructor_direction"
     );

     const float half_pi = 1.57079632679f;
     check_vec4(
          mat4_mul_vec4(rotation_x_constructor(half_pi), (struct Vector4){0.0f, 1.0f, 0.0f, 1.0f}),
          (struct Vector4){0.0f, 0.0f, 1.0f, 1.0f},
          "rotation_x_constructor"
     );
     check_vec4(
          mat4_mul_vec4(rotation_y_constructor(half_pi), (struct Vector4){0.0f, 0.0f, 1.0f, 1.0f}),
          (struct Vector4){-1.0f, 0.0f, 0.0f, 1.0f},
          "rotation_y_constructor"
     );
     check_vec4(
          mat4_mul_vec4(rotation_z_constructor(half_pi), (struct Vector4){1.0f, 0.0f, 0.0f, 1.0f}),
          (struct Vector4){0.0f, 1.0f, 0.0f, 1.0f},
          "rotation_z_constructor"
     );

     struct Matrix scale = scale_constructor(2.0f, 3.0f, 4.0f);
     check_vec4(
          mat4_mul_vec4(scale, (struct Vector4){1.0f, 1.0f, 1.0f, 1.0f}),
          (struct Vector4){2.0f, 3.0f, 4.0f, 1.0f},
          "scale_constructor"
     );
}

static void test_camera_suite(void) {
     printf("\n[SUITE] Camera\n");

     check_mat4(
          lookAt(
               (struct Vector3){0.0f, 0.0f, 0.0f},
               (struct Vector3){0.0f, 0.0f, -1.0f},
               (struct Vector3){0.0f, 1.0f, 0.0f}
          ),
          (struct Matrix){
               .m = {
                    {1.0f, 0.0f, 0.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f, 0.0f},
                    {0.0f, 0.0f, 0.0f, 1.0f},
               }
          },
          "lookAt_identity_view"
     );

     struct Matrix translated_view = lookAt(
          (struct Vector3){0.0f, 0.0f, 5.0f},
          (struct Vector3){0.0f, 0.0f, 0.0f},
          (struct Vector3){0.0f, 1.0f, 0.0f}
     );

     check_mat4(
          translated_view,
          (struct Matrix){
               .m = {
                    {1.0f, 0.0f, 0.0f, 0.0f},
                    {0.0f, 1.0f, 0.0f, 0.0f},
                    {0.0f, 0.0f, 1.0f, -5.0f},
                    {0.0f, 0.0f, 0.0f, 1.0f},
               }
          },
          "lookAt_translated_view"
     );

     check_vec4(
          mat4_mul_vec4(
               translated_view,
               (struct Vector4){0.0f, 0.0f, 0.0f, 1.0f}
          ),
          (struct Vector4){0.0f, 0.0f, -5.0f, 1.0f},
          "lookAt_transforms_target_to_camera_space"
     );
}

static void test_projection_suite(void) {
     printf("\n[SUITE] Projection\n");

     const float fov = 1.57079632679f;
     const float aspect = 1.0f;
     const float near = 1.0f;
     const float far = 10.0f;
     struct Matrix p = perspective(fov, aspect, near, far);

     check_float(p.m[0][0], 1.0f, "perspective_m00");
     check_float(p.m[1][1], 1.0f, "perspective_m11");
     check_float(p.m[2][2], -11.0f / 9.0f, "perspective_m22");
     check_float(p.m[2][3], -20.0f / 9.0f, "perspective_m23");
     check_float(p.m[3][2], -1.0f, "perspective_m32");

     check_float(p.m[0][1], 0.0f, "perspective_zero_m01");
     check_float(p.m[1][0], 0.0f, "perspective_zero_m10");
     check_float(p.m[3][3], 0.0f, "perspective_zero_m33");

     struct Vector4 center_clip = mat4_mul_vec4(p, (struct Vector4){0.0f, 0.0f, -5.0f, 1.0f});
     check_float(center_clip.x / center_clip.w, 0.0f, "perspective_center_ndc_x");
     check_float(center_clip.y / center_clip.w, 0.0f, "perspective_center_ndc_y");
     check_float(center_clip.z / center_clip.w, 0.7777778f, "perspective_center_ndc_z");

     struct Vector4 near_right_clip = mat4_mul_vec4(p, (struct Vector4){1.0f, 0.0f, -1.0f, 1.0f});
     check_float(near_right_clip.x / near_right_clip.w, 1.0f, "perspective_near_right_ndc_x");

     struct Vector4 near_top_clip = mat4_mul_vec4(p, (struct Vector4){0.0f, 1.0f, -1.0f, 1.0f});
     check_float(near_top_clip.y / near_top_clip.w, 1.0f, "perspective_near_top_ndc_y");
}

int main(void) {
     printf("Phase 1 Math Test Suite\n");
     printf("=======================\n");

     test_vector3_suite();
     test_vector4_suite();
     test_matrix_suite();
     test_camera_suite();
     test_projection_suite();

     printf("\n[SUMMARY]\n");
     printf("Checks run : %d\n", g_total_checks);
     printf("Checks pass: %d\n", g_total_checks - g_failed_checks);
     printf("Checks fail: %d\n", g_failed_checks);

     return (g_failed_checks == 0) ? 0 : 1;
}