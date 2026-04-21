#ifndef SHADOW_H
#define SHADOW_H

#include "../../../shared/framebuffer/framebuffer.h"
#include "../../../shared/math/mat/matrix.h"
#include "../../../shared/geometry/geometry.h"
#include "../../../shared/lighting/lighting.h"
#include "../../../shared/math/projection/projection.h"
#include "../../../shared/math/camera/camera.h"

// — wraps a depth-only framebuffer + light MVP matrix
struct ShadowMap {
    struct Framebuffer fb;
    struct Matrix light_mvp;
};

struct ShadowMap shadow_map_create(); // allocates the shadow framebuffer

/*
Build the light's view and projection matrices:
- `light_view = lookAt(light_position, scene_center, world_up)`
- `light_proj = orthographic(left, right, bottom, top, near, far)`
- `light_mvp = light_proj * light_view * model`

Transform all triangles through `light_mvp`. Rasterize each triangle into the shadow framebuffer — but only update the depth buffer, skip color entirely.

Store the `light_mvp` matrix in the `ShadowMap` struct — you'll need it in Pass 2.
*/
void shadow_map_render( struct ShadowMap *shadow_map,
                        struct Vector3 *world_verts,     // world-space vertices
                        int num_verts,
                        const int tri_indices[][3],      // which vertices form each triangle
                        int num_triangles,
                        struct DirectionalLight light,
                        struct Vector3 scene_center); // renders scene into shadow map from light's POV

void shadow_map_destroy(struct ShadowMap *shadow_map);

float compute_shadow(struct ShadowMap *shadow_map, struct Vector3 world_pos, float bias);

#endif