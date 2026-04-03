/*
 * main.c -- test suite for graphics handling
 */

#include "./vector.h"
#include <string.h>
#include <stdio.h>

void print_vec3(struct Vector3 vec3){
     printf("x=%f, y=%f, z=%f\n", vec3.x, vec3.y, vec3.z);
}

void print_vec4(struct Vector4 vec4){
     printf("x=%f, y=%f, z=%f, w=%f\n", vec4.x, vec4.y, vec4.z, vec4.w);
}

int main() {
    printf("Starting tests...\n");

    struct Vector3 vec3_a;
    struct Vector3 vec3_b;

    struct Vector4 vec4_a;
    struct Vector4 vec4_b;

    vec3_a.x = 1;
    vec3_a.y = 2;
    vec3_a.z = 3;

    vec3_b.x = 3;
    vec3_b.y = 2;
    vec3_b.z = 1;

    vec4_a.x = 1;
    vec4_a.y = 2;
    vec4_a.z = 3;
    vec4_a.w = 0;

    vec4_b.x = 3;
    vec4_b.y = 2;
    vec4_b.z = 1;
    vec4_b.w = 0;

    printf("Starting Vectors: \n");
    printf("Vector3 A: ");
    print_vec3(vec3_a);
    printf("Vector3 B: ");
    print_vec3(vec3_b);
    printf("Vector4 A: ");
    print_vec4(vec4_a);
    printf("Vector4 B: ");
    print_vec4(vec4_b);

    printf("\n");
    printf("Addition:\n");
    printf("v3: ");
    print_vec3(v3_addition(vec3_a, vec3_b));

    printf("\n");
    printf("Addition:\n");
    printf("v4: ");
    print_vec4(v4_addition(vec4_a, vec4_b));

    printf("\n");
    printf("Subtraction:\n");
    printf("v3: ");
    print_vec3(v3_subtraction(vec3_a, vec3_b));

    printf("\n");
    printf("Subtraction:\n");
    printf("v4: ");
    print_vec4(v4_subtraction(vec4_a, vec4_b));


    return 0;
}