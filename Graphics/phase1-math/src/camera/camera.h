#ifndef CAMERA_H
#define CAMERA_H

#include "../mat/matrix.h"
#include "../vec/vector.h"

struct Matrix lookAt(struct Vector3 eye, struct Vector3 target, struct Vector3 up);

#endif