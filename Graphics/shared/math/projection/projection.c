#include "./projection.h"

struct Matrix perspective(float fov, float aspect, float near, float far){
    struct Matrix perspective;

    // initialize matrix to all 0's
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            perspective.m[i][j] = 0;
        }
    }

    // find half_width and half_height 
    // Geometrically, half height is equivalent to tan(fov/2) * near
    //     /|
    //    / |
    //   /| |  angle of right triangle = fov/2. half height = opposite, tan(angle) = opposite/adjacent = half_height/near, 
    //  /_|_|  where near = distance from camera to near plane

    // when mapping x coordinates to -1 <= x <= 1, we want these values in between the width values, so divide by half_width

    perspective.m[0][0] = 1 / (tanf(fov / 2) * aspect); // divide by max width
    perspective.m[1][1] = 1 / (tanf(fov / 2)); // divide by max height

    /*
    calculations for the Z adjustment is much more complicated. After we find the adjusted Z, we divide by the old Z value to find the "depth"
    This would cause Z to cancel out and everything would have the same perceived depth. So, we need to include an additional factor in [2][3]

    Unlike X and Y, we will not only use [i][i]

    Unfortunately, there is not a smooth intuitional way to think about the following formula, as it is more of an algebraic derivation than a geometric one

    In any case, the resulting values are as follows:
    */

    perspective.m[2][2] = -(far+near)/(far-near);
    perspective.m[2][3] = -(2*far*near)/(far-near);

    perspective.m[3][2] = -1; // store the old value of Z in w, negating it to match the "reverse" depth due to the camera's perspective.

    return perspective;
}

// Divide each coordinate (x,y,z) by the stored w value. This restores a perceived "depth" to the projection
struct Vector4 perspective_divide(struct Vector4 clip){
    if (clip.w == 0) {
        struct Vector4 zero = {0, 0, 0, 0};
        return zero;
    }
    struct Vector4 divide;
    divide.x = clip.x / clip.w;
    divide.y = clip.y / clip.w;
    divide.z = clip.z / clip.w;
    divide.w = clip.w;
    return divide;
}

// Map to viewport pixel coordinates. z is unused for now, but will later be used for the depth buffer
struct Vector4 viewport (struct Vector4 ndc, float height, float width){
    struct Vector4 view;
    view.x = (ndc.x + 1) / 2 * width;
    view.y = (1 - ndc.y) / 2 * height;
    view.z = ndc.z; // stored for later use
    view.w = ndc.w;
    return view;
}

/*
Unlike perspective, orthographic projection doesn't divide by depth — 
it just scales and translates the view volume into the NDC cube. There's no `-1` in row 3 column 2 and no `w` trick.

The matrix maps:
- x from `[left, right]` to `[-1, 1]`
- y from `[bottom, top]` to `[-1, 1]`
- z from `[-near, -far]` to `[-1, 1]`
*/

struct Matrix orthographic(float left, float right, float bottom, float top, float near, float far){
    struct Matrix ortho;

    // initialize matrix to all 0's
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            ortho.m[i][j] = 0.0f;
        }
    }

    // output = (2/(max-min)) * input - (max+min)/(max-min)

    ortho.m[0][0] = 2.0f / (right - left);
    ortho.m[1][1] = 2.0f / (top - bottom);
    ortho.m[2][2] = -2.0f / (far - near);
    ortho.m[3][3] = 1.0f;

    ortho.m[0][3] = -1.0f * (right + left) / (right - left);
    ortho.m[1][3] = -1.0f * (top + bottom) / (top - bottom);
    ortho.m[2][3] = -1.0f * (far + near) / (far - near);

    return ortho;
}