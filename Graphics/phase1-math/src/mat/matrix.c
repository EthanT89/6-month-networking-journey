#include "./matrix.h"

struct Matrix mat4_mul(struct Matrix a, struct Matrix b){
    /*
    
    2x3         3x2         2x2
    | a b c |   | g h |     | a*g + b*i + c*k       a*h + b*j + c*l |     
    | d e f | x | i j | ->  | d*g + e*i + f*k       d*h + e*j + f*l |
                | k l |

       0 1 2 3            0 1 2 3
   0 | a b c d |      0 | a b c d |
   1 | e f g h |  \/  1 | e f g h |
   2 | i j k l |  /\  2 | i j k l |
   3 | m n o p |      3 | m n o p |

    */
    struct Matrix res;

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            res.m[i][j] = 0;

            for (int k = 0; k < 4; k++){
                res.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }

    return res;
}

struct Vector4 mat4_mul_vec4(struct Matrix a, struct Vector4 b){
    struct Vector4 res;

    res.x = a.m[0][0]*b.x + a.m[0][1]*b.y + a.m[0][2]*b.z + a.m[0][3]*b.w;
    res.y = a.m[1][0]*b.x + a.m[1][1]*b.y + a.m[1][2]*b.z + a.m[1][3]*b.w;
    res.z = a.m[2][0]*b.x + a.m[2][1]*b.y + a.m[2][2]*b.z + a.m[2][3]*b.w;
    res.w = a.m[3][0]*b.x + a.m[3][1]*b.y + a.m[3][2]*b.z + a.m[3][3]*b.w;

    return res;
}

struct Matrix identity_matrix_constructor(){
    struct Matrix identity;

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j ++){
            if (i == j){
                identity.m[i][j] = 1;
            } else {
                identity.m[i][j] = 0;
            }
        }
    }

    return identity;
}

struct Matrix transpose(struct Matrix a){
    struct Matrix transposed;

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j ++){
            transposed.m[i][j] = a.m[j][i];
        }
    }

    return transposed;
}

struct Matrix translation_constructor(float tx, float ty, float tz){
    struct Matrix translation;

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j ++){
            if (i == j) translation.m[i][j] = 1; // keep original values
            else translation.m[i][j] = 0;
        }
    }

    translation.m[0][3] = tx;
    translation.m[1][3] = ty;
    translation.m[2][3] = tz;

    return translation;

    /*
    Resulting matrix:

    
       0 1 2 3       
   0 | 1 0 0 tx |      
   1 | 0 1 0 ty | - > t gets multiplied by the w component. If w = 0 (directional vector), the vector is not translated at all. (equivalent to identity vector)   
   2 | 0 0 1 tz | 
   3 | 0 0 0 1  |  
    
    */
}

struct Matrix rotation_x_constructor(float angle){
    struct Matrix rotation;

    /*
    
        0  1  2  3    
   0 |  1  0       0       0  |   | 0 |     | 0 |
   1 |  0  cos(a) -sin(a)  0  |   | 0 | - > |-1 |
   2 |  0  sin(a)  cos(a)  0  | X | 1 |     | 0 |
   3 |  0  0       0       1  |   | 0 |     | 0 |

    */

    // initialize to 0's
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j ++){
            rotation.m[i][j] = 0;
        }
    }

    rotation.m[0][0] = 1; // x axis unchanged
    rotation.m[3][3] = 1; // w val unchanged
    rotation.m[1][1] = cos(angle); // derived from manual example with 180 degree turn
    rotation.m[2][2] = cos(angle);
    rotation.m[1][2] = -sin(angle);
    rotation.m[2][1] = sin(angle);

    return rotation;
}

struct Matrix rotation_y_constructor(float angle){
    struct Matrix rotation;

    /*
    
        0  1  2  3    
   0 |  cos(a)  0      -sin(a)  0  |   | 0 |     |-1 |
   1 |  0       1       0       0  |   | 0 | - > | 0 |
   2 |  sin(a)  0       cos(a)  0  | X | 1 |     | 0 |
   3 |  0       0       0       1  |   | 0 |     | 0 |

    */

    // initialize to 0's
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j ++){
            rotation.m[i][j] = 0;
        }
    }

    rotation.m[1][1] = 1; // x axis unchanged
    rotation.m[3][3] = 1; // w val unchanged
    rotation.m[0][0] = cos(angle); // derived from manual example with 180 degree turn
    rotation.m[2][2] = cos(angle);
    rotation.m[0][2] = -sin(angle);
    rotation.m[2][0] = sin(angle);

    return rotation;
}

struct Matrix rotation_z_constructor(float angle){
    struct Matrix rotation;

    // initialize to 0's
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j ++){
            rotation.m[i][j] = 0;
        }
    }

    rotation.m[2][2] = 1; // x axis unchanged
    rotation.m[3][3] = 1; // w val unchanged
    rotation.m[1][1] = cos(angle); // derived from manual example with 180 degree turn
    rotation.m[0][0] = cos(angle);
    rotation.m[1][0] = sin(angle);
    rotation.m[0][1] = -sin(angle);

    return rotation;
}

struct Matrix scale_constructor(float sx, float sy, float sz){
    struct Matrix scaled;

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j ++){
            if (i == j){
                if (i == 0) scaled.m[i][j] = sx;
                else if (i == 1) scaled.m[i][j] = sy;
                else if (i == 2) scaled.m[i][j] = sz;
                else scaled.m[i][j] = 1;
            } else {
                scaled.m[i][j] = 0;
            }
        }
    }

    return scaled;
}