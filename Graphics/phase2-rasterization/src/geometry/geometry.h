#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "../../../phase1-math/src/vec/vector.h"

struct Triangle {
    struct Vector3 v[3];   // vertices in clip space
    unsigned char r, g, b; // flat color for now
};

int is_back_face(struct Triangle *t); // returns 1 if back-facing

float signed_area(struct Vector3 A, struct Vector3 B, struct Vector3 C );

#endif