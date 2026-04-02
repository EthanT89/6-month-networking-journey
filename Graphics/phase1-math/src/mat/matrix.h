/*
 * matrix.h -- simple matrix utils
 */

#include "../vec/vector.h"

struct Matrix {
    float m[4][4];
};

struct Matrix mat4_mul(struct Matrix a, struct Matrix b);

struct Vector4 mat4_mul_vec4(struct Matrix a, struct Vector4 b);

struct Matrix identity_matrix_constructor();

struct Matrix transpose(struct Matrix a);

struct Matrix translation_constructor(float translation);

struct Matrix rotation_x_constructor(float angle);

struct Matrix rotation_y_constructor(float angle);

struct Matrix rotation_z_constructor(float angle);

struct Matrix scale_constructor(float scale);

/*

| Matrix | What it does |
|---|---|
| Identity | Does nothing — `M * v = v` |
| Scale | Stretches space along axes |
| Rotation (X/Y/Z) | Rotates space around an axis |
| Translation | Moves points (requires 4×4 homogeneous) |
| Transpose | Flips rows and columns — useful for inverting orthogonal matrices |

Things to implement:

mat4 struct: float m[4][4]
Identity matrix constructor
Matrix multiplication (mat4_mul)
Matrix-vector multiplication (mat4_mul_vec4)
Transpose
Translation, rotation (X, Y, Z), scale constructors

*/