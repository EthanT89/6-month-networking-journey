#ifndef LIGHTING_H
#define LIGHTING_H

#include "../math/vec/vector.h"
#include <math.h>

struct DirectionalLight {
    struct Vector3 direction;  // unit vector pointing toward the light
    float intensity;
    unsigned char r, g, b;
};

struct Vector3 compute_face_normal(struct Vector4 v0, struct Vector4 v1, struct Vector4 v2);

float compute_diffuse(struct Vector3 normal, struct DirectionalLight light);

float compute_specular(struct Vector3 normal, struct Vector3 light_dir,
                       struct Vector3 view_dir, float shininess);

#endif