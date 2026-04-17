#include "./lighting.h"

/*
 * computes the face normal of a triangle using counter-clockwise positive rotation
 */
struct Vector3 compute_face_normal(struct Vector3 v0, struct Vector3 v1, struct Vector3 v2)
{
    // using counter-clockwise positive convention,
    // we define the two vectors to be originating
    // from the first vertex, v0
    struct Vector3 a = v3_subtraction(v1, v0);
    struct Vector3 b = v3_subtraction(v2, v0);

    struct Vector3 normal = v3_norm(v3_cross(a, b)); // normalize the cross product
    return normal;
}


float compute_diffuse(struct Vector3 normal, struct DirectionalLight light)
{
    /*
     * Diffuse objects reflect light in all directions. Thus, it's appearance is independent
     * of the viewing angle. That is why this calculation does not include the camera position.
     * 
     * Combined with specular, we can scheme a "general" appearance combined with the view dependent
     * appearance (specular).
     * 
     * Reflective objects appear differently depending on the view angle, like how a mirror reflects
     * differently when you move around. Somewhat simple concept, but very difficult to simulate accurately
     */
    float diffuse = fmax(0.0f, v3_dot(normal, light.direction)) * light.intensity;
    return diffuse;
}

float compute_specular(struct Vector3 normal, struct Vector3 light_dir,
                       struct Vector3 view_dir, float shininess)
{
    /*
     * "halfway" represents the vector in between the viewing direction and light direction.
     *
     * This is useful to us because if the light is reflected perfectly into the camera, that means
     * the angle between the light direction and the surface normal is equivalent to the angle
     * between the viewing angle and the surface normal.
     * 
     * Thus, that is why we take the dot product of the halfway and the normal. Theoretically,
     * if the light is perfectly reflecting into the camera, the midpoint should point in the same
     * direction as the normal, and the dot product should = 1.
     * 
     * This visual should help the understanding:
     * 
     *    *camera*   *n*  *light source*
     *            \   |   /
     *             \  |  /  -- light reflects at an angle opposite to the entry angle
     *              \ | /   -- thus, directly into the camera, and the "midpoint"
     *         ______\|/____-- is equal to the normal (*n*)
     */
    struct Vector3 halfway = v3_norm(v3_addition(light_dir, view_dir));
    float specular = powf(fmax(0.0f, v3_dot(normal, halfway)), shininess);

    return specular;
}