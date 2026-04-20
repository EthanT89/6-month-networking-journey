#ifndef PROJECTION_H
#define PROJECTION_H

#include "../vec/vector.h"
#include "../mat/matrix.h"

struct Matrix perspective(float fov, float aspect, float near, float far);

struct Vector4 perspective_divide(struct Vector4 clip);

struct Vector4 viewport (struct Vector4 ndc, float height, float width);

struct Matrix orthographic(float left, float right, float bottom, float top, float near, float far);

#endif