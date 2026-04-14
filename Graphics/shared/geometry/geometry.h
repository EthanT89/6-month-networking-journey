#ifndef GEOMETRY2_H
#define GEOMETRY2_H

#include "../math/vec/vector.h"


struct Triangle {
    struct Vector4 v[3];
    struct Vector3 normal;     // face normal in world space
    float brightness[3];
    unsigned char r, g, b;
};

int is_back_face(struct Triangle *t); // returns 1 if back-facing

float signed_area(struct Vector3 A, struct Vector3 B, struct Vector3 C );

struct Vector3 get_triangle_midpoint(struct Triangle *t);

#endif