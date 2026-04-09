#include "./geometry.h"

int is_back_face(struct Triangle *t){
    float area = signed_area(t->v[0], t->v[1], t->v[2]);

    return area > 0 ? 1 : 0;
}

float signed_area(struct Vector3 A, struct Vector3 B, struct Vector3 C ){
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
    return (B.x - A.x) * (C.y - A.y) - (C.x - A.x) * (B.y - A.y);
}