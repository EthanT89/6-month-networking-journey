/*
 * vector.h -- util file for managing 3 and 4 dimensional vectors
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include <stdlib.h>

struct Vector2 {
    float x;
    float y;
};

struct Vector3 {
    float x, y, z;
};

struct Vector4 {
    float x, y, z, w;
};

struct Vector3 v3_addition (struct Vector3 a, struct Vector3 b);

struct Vector4 v4_addition (struct Vector4 a, struct Vector4 b);

struct Vector3 v3_subtraction (struct Vector3 a, struct Vector3 b);

struct Vector4 v4_subtraction (struct Vector4 a, struct Vector4 b);

struct Vector3 v3_scalar_mult (struct Vector3 a, float b);

struct Vector4 v4_scalar_mult (struct Vector4 a, float b);

float v3_dot (struct Vector3 a, struct Vector3 b);

float v4_dot (struct Vector4 a, struct Vector4 b);

struct Vector3 v3_cross (struct Vector3 a, struct Vector3 b);

struct Vector4 v4_cross (struct Vector4 a, struct Vector4 b);

struct Vector3 v3_norm (struct Vector3 a);

struct Vector4 v4_norm (struct Vector4 a);

float v3_magnitude (struct Vector3 a);

float v4_magnitude (struct Vector4 a);

#endif