/*
 * vector.c -- simple utils for 3 and 4 dimensional vectors
 */

 #include "./vector.h"

struct Vector3 v3_addition (struct Vector3 a, struct Vector3 b){
    struct Vector3 res;
    res.x = a.x + b.x;
    res.y = a.y + b.y;
    res.z = a.z + b.z;

    return res;
}

struct Vector4 v4_addition (struct Vector4 a, struct Vector4 b){
    struct Vector4 res;
    res.x = a.x + b.x;
    res.y = a.y + b.y;
    res.z = a.z + b.z;
    res.w = a.w + b.w;

    return res;
}

struct Vector3 v3_subtraction (struct Vector3 a, struct Vector3 b){
    struct Vector3 res;
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    res.z = a.z - b.z;

    return res;
}

struct Vector4 v4_subtraction (struct Vector4 a, struct Vector4 b){
    struct Vector4 res;
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    res.z = a.z - b.z;
    res.w = a.w - b.w;

    return res;
}

struct Vector3 v3_scalar_mult (struct Vector3 a, float b){
    struct Vector3 res;
    res.x = a.x * b;
    res.y = a.y * b;
    res.z = a.z * b;

    return res;
}

struct Vector4 v4_scalar_mult (struct Vector4 a, float b){
    struct Vector4 res;
    res.x = a.x * b;
    res.y = a.y * b;
    res.z = a.z * b;
    res.w = a.w * b;

    return res;
}

float v3_dot_product (struct Vector3 a, struct Vector3 b){
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float v4_dot_product (struct Vector4 a, struct Vector4 b){
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

struct Vector3 v3_cross_product (struct Vector3 a, struct Vector3 b){
    struct Vector3 res;
    res.x = a.y * b.z - b.y * a.z;
    res.y = a.z * b.x - b.z * a.x;
    res.z = a.x * b.y - b.x * a.y;

    return res;
}

struct Vector4 v4_cross_product (struct Vector4 a, struct Vector4 b){
    struct Vector4 res;
    res.x = a.y * b.z - b.y * a.z;
    res.y = a.z * b.x - b.z * a.x;
    res.z = a.x * b.y - b.x * a.y;
    res.w = 0;

    return res;
}

struct Vector3 v3_normalize (struct Vector3 a){
    float len = v3_magnitude(a);
    if (len == 0) return a;
    a.x = a.x / len;
    a.y = a.y / len;
    a.z = a.z / len;
    return a;
}

struct Vector4 v4_normalize (struct Vector4 a){
    float len = v4_magnitude(a);
    if (len == 0) return a;
    a.x = a.x / len;
    a.y = a.y / len;
    a.z = a.z / len;
    return a;
}

float v3_magnitude (struct Vector3 a){
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

float v4_magnitude (struct Vector4 a){
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}
