#include "./shadow.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct ShadowMap shadow_map_create(){
    struct Framebuffer depth_fb = fb_create_depth_only(1028, 1028);
    struct Matrix mat;

    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            mat.m[i][j] = 0.0f;
        }
    }

    struct ShadowMap shadow_map;
    shadow_map.fb = depth_fb;
    shadow_map.light_mvp = mat;

    return shadow_map;
}

void shadow_map_render( struct ShadowMap *shadow_map,
                        struct Vector3 *world_verts,     // world-space vertices
                        int num_verts,
                        const int tri_indices[][3],      // which vertices form each triangle
                        int num_triangles,
                        struct DirectionalLight light,
                        struct Vector3 scene_center)
{
    struct Vector3 light_pos    = v3_subtraction(scene_center,
                                    v3_scalar_mult(light.direction, 10.0f));
    struct Vector3 light_up     = {0.0f, 1.0f, 0.0f};
    struct Matrix light_view    = lookAt(light_pos, scene_center, light_up);
    struct Matrix ortho         = orthographic(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 50.0f);
    struct Matrix light_vp     = mat4_mul(ortho, light_view);

    struct Vector3 clip_verts[num_verts];
    for (int vi = 0; vi < num_verts; vi++){
        struct Vector4 world_pos = {world_verts[vi].x, world_verts[vi].y, world_verts[vi].z, 1.0f};
        struct Vector4 clip = mat4_mul_vec4(light_vp, world_pos);
        struct Vector4 vp = viewport(clip, shadow_map->fb.height, shadow_map->fb.width);
        clip_verts[vi] = (struct Vector3){vp.x, vp.y, vp.z};
    }

    shadow_map->light_mvp = light_vp;

    // render triangles depth-only
    for (int i = 0; i < num_triangles; i++){

        struct Vector3 v0 = clip_verts[tri_indices[i][0]];
        struct Vector3 v1 = clip_verts[tri_indices[i][1]];
        struct Vector3 v2 = clip_verts[tri_indices[i][2]];

        // 1. **Bounding box** — find min/max x and y across all three vertices. Clamp to framebuffer bounds.
        int minx = MIN(v0.x, MIN(v1.x, v2.x));
        int miny = MIN(v0.y, MIN(v1.y, v2.y));
        int maxx = MAX(v0.x, MAX(v1.x, v2.x));
        int maxy = MAX(v0.y, MAX(v1.y, v2.y));

        minx = MAX(minx, 0);
        miny = MAX(miny, 0);
        maxx = MIN(maxx, shadow_map->fb.width - 1);
        maxy = MIN(maxy, shadow_map->fb.height - 1);

        // 2. **Barycentric coordinates** — for each pixel in the bounding box, compute the barycentric weights. Use the signed area method.
        float total_area = signed_area(v0, v1, v2);
        if (fabsf(total_area) < 1e-6f) continue;

        for (int x = minx; x <= maxx; x++){
            for (int y = miny; y <= maxy; y++){

                struct Vector3 p = {x,y,0}; // initialize to 0 depth because we don't have that yet.

                // unnormalized weights (before dividing by total_area)
                float w0 = signed_area(p, v1, v2);
                float w1 = signed_area(v0, p, v2);
                float w2 = signed_area(v0, v1, p);

                // 3. **Inside test** — if all three weights are ≥ 0, the pixel is inside the triangle.
                if (w0 > 0 || w1 > 0 || w2 > 0) continue; // inside check, discard if outside

                // normalized weights (after dividing)
                float alpha = w0 / total_area;
                float beta = w1 / total_area;
                float gamma = w2 / total_area;

                // 4. **Depth interpolation** — interpolate the depth value using barycentric weights. Use perspective-correct interpolation.
                p.z = alpha * v0.z + beta * v1.z + gamma * v2.z; // naive approach, currently not using w_clipspace
                
                // 5. **Depth test** — compare interpolated depth against the depth buffer. Only proceed if closer.
                if ( fb_depth_test(&shadow_map->fb, x, y, p.z) == 0) continue;
            }
        }
    }
}

void shadow_map_destroy(struct ShadowMap *shadow_map){
    fb_destroy(&(shadow_map->fb));
}

float compute_shadow(struct ShadowMap *shadow_map, struct Vector3 world_pos, float bias){
    struct Vector4 world_pos_v4 = {world_pos.x, world_pos.y, world_pos.z, 1.0f};
    struct Vector4 clip = mat4_mul_vec4(shadow_map->light_mvp, world_pos_v4);

    float ndc_x = clip.x / clip.w;
    float ndc_y = clip.y / clip.w;
    float ndc_z = clip.z / clip.w;

    // u = (x_ndc + 1) / 2
    float u = (ndc_x + 1) / 2;
    float v = (ndc_y + 1) / 2;

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f) return 0.0f;

    float depth = fb_sample_depth(&shadow_map->fb, u, v);
    
    if (ndc_z > depth + bias){
        return 1.0f;  // in shadow
    }
    return 0.0f;  // lit
}