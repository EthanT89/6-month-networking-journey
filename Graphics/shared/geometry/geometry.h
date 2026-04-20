#ifndef GEOMETRY2_H
#define GEOMETRY2_H

#include "../math/vec/vector.h"


struct Triangle {
    struct Vector3 v_cam[3]; // vertex camera-space data
    struct Vector3 v_world[3]; // vertex world-space data
    float clip_w[3]; // per-vertex clipspace w value
    struct Vector2 uv[3]; // per-vertex uv texture coordinates
    struct Vector3 normal; // face normal in world space

    // rendering properties
    float brightness[3]; // per-vertex brightness
    unsigned char r, g, b; // rgb values, unused for texture mapping, TBD what to do with them now that we are using texture maps
};

int is_back_face(struct Triangle *t); // returns 1 if back-facing

float signed_area(struct Vector3 A, struct Vector3 B, struct Vector3 C );

struct Vector3 get_triangle_midpoint(struct Triangle *t);

#endif