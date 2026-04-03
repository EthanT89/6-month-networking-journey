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

    perspective.m[0][0] = 1 / (tan(fov / 2) * aspect); // divide by max height
    perspective.m[1][1] = 1 / (tan(fov / 2)); // divide by max width

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