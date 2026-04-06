#ifndef PROJECTION_H
#define PROJECTION_H

#include "../vec/vector.h"
#include "../mat/matrix.h"

struct Matrix perspective(float fov, float aspect, float near, float far);

struct Vector3 perspective_divide(struct Vector4 clip);

struct Vector3 viewport (struct Vector3 ndc, float height, float width);

#endif