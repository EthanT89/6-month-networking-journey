#include "./geometry.h"

int is_back_face(struct Triangle *t){
    // signed_area = (B.x - A.x) * (C.y - A.y) - (C.x - A.x) * (B.y - A.y)
    /*
     * Triangle vertices:
     *
     *          C
     *        / |
     *       /  |   
     *      A___B  
     * 
     * Cross product of two vectors:
     *  | A.x A.y |
     *  | B.x B.y |
     * 
     *  Area = A.x * B.y - A.y * B.x
     * 
     *  Therefore, in the above signed_area function,
     *  A.x = B.x - A.x
     *  A.y = C.x - A.x
     *  B.x = C.y - A.y
     *  B.y = B.y - A.y
     * 
     *  Meaning that A was subtracted from both B and C to create two vectors, both originating from A, towards B and C, respectively.
     * 
     *         C
     *        ^ 
     *       /     
     *      A->B
     * 
     *  As such ^^^  
     */

    // So, 
    float signed_area = ( t->v[1].x - t->v[0].x ) * ( t->v[2].y - t->v[0].y ) - ( t->v[2].x - t->v[0].x ) * ( t->v[1].y - t->v[0].y );

    return signed_area < 0 ? 1 : 0;
}