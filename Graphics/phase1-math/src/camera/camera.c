#include "./camera.h"

struct Matrix lookAt(struct Vector3 eye, struct Vector3 target, struct Vector3 up){
    // need to define the up, backward, and right vectors.

    struct Vector3 true_up;
    struct Vector3 backward;
    struct Vector3 right;

    /*
    
        eye = (0, 0, 5)
        target = (0, 0, 0)
        up = (0, 1, 0)

    */


    // First, find backward. Can get vector by subtracting (eye and target are points, not directions)
    backward = v3_norm(v3_subtraction(eye, target)); // -> { 0, 0, 1 }
    right = v3_norm(v3_cross(up, backward)); // { 0, 1, 0 } X { 0, 0, 1 } -> { 1, 0, 0}
    true_up = v3_cross(backward, right); // { 0, 0, 1 } X { 1, 0, 0 } -> { 0, 1, 0 } 

    struct Matrix lookAt;
    lookAt.m[0][0] = right.x;
    lookAt.m[1][0] = true_up.x;
    lookAt.m[2][0] = backward.x;
    lookAt.m[3][0] = 0;

    lookAt.m[0][1] = right.y;
    lookAt.m[1][1] = true_up.y;
    lookAt.m[2][1] = backward.y;
    lookAt.m[3][1] = 0;

    lookAt.m[0][2] = right.z;
    lookAt.m[1][2] = true_up.z;
    lookAt.m[2][2] = backward.z;
    lookAt.m[3][2] = 0;

    lookAt.m[0][3] = -v3_dot(right, eye); // -> 0
    lookAt.m[1][3] = -v3_dot(true_up, eye); // -> 0 
    lookAt.m[2][3] = -v3_dot(backward, eye); // -> -5
    lookAt.m[3][3] = 1;

    /*
    
    final matrix:

    | 1 0 0  0 |
    | 0 1 0  0 |
    | 0 0 1 -5 |
    | 0 0 0  1 |
    
    */

    return lookAt;
}