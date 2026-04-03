#ifndef PROJECTION_H
#define PROJECTION_H

#include "../vec/vector.h"
#include "../mat/matrix.h"

struct Matrix perspective(float fov, float aspect, float near, float far);

#endif