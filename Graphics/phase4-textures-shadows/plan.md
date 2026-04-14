# Phase 4 — Textures & Shadows: Plan

> **Duration:** ~2 weeks of deep work
> **Language:** C
> **Depends on:** All of Phase 1, 2, and 3
> **Goal:** Map images onto geometry with correct perspective, and cast accurate shadows using a two-pass shadow mapping technique.

---

## Overview

Phase 4 has two independent halves that combine at the end:

- **Textures** — UV coordinates, perspective-correct interpolation, texture sampling
- **Shadows** — shadow map generation, shadow testing, bias correction, PCF softening

You implement them in order. Textures first because they extend the rasterizer in a way that's self-contained. Shadows second because they require a second render pass that builds on the full pipeline.

---

## File Structure

```
phase4-textures-shadows/
├── README.md
├── log/
└── src/
    ├── main.c
    ├── texture/
    │   ├── texture.h
    │   ├── texture.c
    │   └── stb_image.h          ← single-header library, drop in here
    └── shadow/
        ├── shadow.h
        └── shadow.c
```

**Shared from previous phases:**
- `phase1-math/src/` — all math (vec, mat, camera, projection)
- `phase2-rasterization/src/framebuffer/` — framebuffer (extended for shadow map)
- `phase2-rasterization/src/raster/` — draw_triangle (extended for UV and shadow)
- `phase2-rasterization/src/geometry/` — Triangle struct (extended for UV and clip_w)
- `phase3-lighting/src/lighting/` — all lighting functions

**Key extensions to existing files:**

The `Triangle` struct needs three new fields:
- UV coordinates per vertex — `struct Vector2 uv[3]`
- Clip-space w values per vertex — `float clip_w[3]`
- Pointer or reference to a texture

`draw_triangle` needs new parameters to support UV interpolation and shadow testing.

---

## Part 1 — Textures

### Step 1 — Vector2 Struct (Day 1)

Add `struct Vector2` to your vector library in `phase1-math/src/vec/`:

**Fields:** `float x, y`

You need this for UV coordinates. Add it to `vector.h` and `vector.c` alongside `Vector3` and `Vector4`. No new operations are required beyond what's already there — UV coordinates are just stored and interpolated, not crossed or dotted.

---

### Step 2 — Texture Infrastructure (Days 1–2)

**`texture.h`** declares:

```
struct Texture        — width, height, pointer to pixel data
texture_load()        — loads an image file using stb_image
texture_sample_nn()   — nearest-neighbor sampling at (u,v)
texture_sample_bl()   — bilinear sampling at (u,v) [stretch goal]
texture_destroy()     — frees pixel data
```

**`texture.c`** implements all of the above.

**Getting `stb_image.h`:**
Download from `github.com/nothings/stb`. Place in `phase4/src/texture/`. In exactly one `.c` file (your `texture.c`), define `STB_IMAGE_IMPLEMENTATION` before including it. Every other file that needs it just includes the header normally.

**`texture_sample_nn` logic:**
- Accept `u` and `v` in `[0,1]`
- Apply wrapping (clamp or repeat) before converting to pixel coordinates
- Convert: `tex_x = (int)(u * (width - 1))`, same for y
- Return the RGB values at that pixel

**`texture_sample_bl` logic (stretch):**
- Find the four surrounding texels
- Compute fractional offsets within the texel cell
- Bilinearly interpolate: blend horizontally first, then vertically

**Checkpoint:** Load a test image, sample it at a grid of UV coordinates, print the RGB values. Verify corners match what you'd expect from the image.

---

### Step 3 — UV Coordinates on the Triangle Struct (Day 3)

Extend the `Triangle` struct:

```
struct Triangle {
    struct Vector3 v[3];
    struct Vector2 uv[3];      ← new
    float clip_w[3];           ← new (for perspective-correct interp)
    struct Vector3 normal;
    float brightness;
    unsigned char r, g, b;
};
```

Define UV coordinates for each face of the cube. For a simple cube, each face maps the full texture from `(0,0)` to `(1,1)`. The UV assignment follows your winding order:

For a counterclockwise triangle `{4, 5, 6}` (front face, bottom-left triangle):
- vertex 4 (bottom-left) → UV `(0, 0)`
- vertex 5 (bottom-right) → UV `(1, 0)`
- vertex 6 (top-right) → UV `(1, 1)`

Work out the UV assignments for all 12 triangles consistently. Each face pair shares UVs that together cover the full texture square.

Store `clip_w[i]` from the clip-space `w` value of each vertex before the perspective divide. You already compute these — just store them alongside the triangle.

**Checkpoint:** Print UV coordinates for all 12 triangles. Each face pair should cover `(0,0)→(1,1)` consistently.

---

### Step 4 — Perspective-Correct UV Interpolation (Days 4–5)

Extend `draw_triangle` to accept a texture and interpolate UVs.

New signature concept:
```
draw_triangle_textured(fb, v0, v1, v2, uv0, uv1, uv2, w0, w1, w2, brightness, tex)
```

Inside the rasterization loop, after computing `alpha`, `beta`, `gamma`:

**Perspective-correct UV interpolation formula:**

```
inv_w = alpha * (1/w0) + beta * (1/w1) + gamma * (1/w2)
u_interp = (alpha * uv0.x/w0 + beta * uv1.x/w1 + gamma * uv2.x/w2) / inv_w
v_interp = (alpha * uv0.y/w0 + beta * uv1.y/w1 + gamma * uv2.y/w2) / inv_w
```

Sample the texture at `(u_interp, v_interp)`. Apply brightness to the sampled color. Write to the framebuffer.

**Checkpoint:** A textured cube with no visible UV warping. The texture should appear correctly mapped on all visible faces, with no distortion at sharp angles.

---

### Step 5 — Textured Lit Cube (Day 6)

Wire textures into the full pipeline from Phase 3:

- Load a texture at startup
- Assign UVs to all 12 triangles
- Store `clip_w` values during the clip-space transform loop
- Call `draw_triangle_textured` instead of `draw_triangle`
- Verify lighting still works — texture color should be modulated by brightness

**Checkpoint:** A textured cube with correct Phong lighting. The texture is visible, lit faces are bright, dark faces show the texture dimly. Specular highlight appears as a bright spot on the texture.

---

## Part 2 — Shadow Mapping

### Step 6 — Orthographic Projection (Days 7–8)

Your `projection.c` already has a `perspective()` function. Add `orthographic()`.

**`orthographic` signature:**
```
struct Matrix orthographic(float left, float right, float bottom, float top, float near, float far)
```

Unlike perspective, orthographic projection doesn't divide by depth — it just scales and translates the view volume into the NDC cube. There's no `-1` in row 3 column 2 and no `w` trick.

The matrix maps:
- x from `[left, right]` to `[-1, 1]`
- y from `[bottom, top]` to `[-1, 1]`
- z from `[-near, -far]` to `[-1, 1]`

**Checkpoint:** Render the cube through an orthographic projection. It should look like a technical drawing — no perspective foreshortening, parallel edges stay parallel.

---

### Step 7 — Shadow Framebuffer (Day 9)

The shadow map is a depth-only framebuffer — you need to render depth into it but you don't need a color buffer.

Add a function to your framebuffer infrastructure:

```
struct Framebuffer fb_create_depth_only(int width, int height)
```

This creates a framebuffer with a depth buffer but either a minimal or null color buffer. The shadow map resolution is typically independent of screen resolution — 1024×1024 is a common starting point.

Add a sampling function:

```
float fb_sample_depth(struct Framebuffer *fb, float u, float v)
```

This converts UV coordinates to pixel coordinates and returns the depth value stored there. This is how Pass 2 reads the shadow map.

---

### Step 8 — Shadow Map Generation (Days 10–11)

**`shadow.h`** declares:

```
struct ShadowMap         — wraps a depth-only framebuffer + light MVP matrix
shadow_map_create()      — allocates the shadow framebuffer
shadow_map_render()      — renders scene into shadow map from light's POV
shadow_map_destroy()     — frees resources
```

**`shadow_map_render` logic:**

Build the light's view and projection matrices:
- `light_view = lookAt(light_position, scene_center, world_up)`
- `light_proj = orthographic(left, right, bottom, top, near, far)`
- `light_mvp = light_proj * light_view * model`

Transform all triangles through `light_mvp`. Rasterize each triangle into the shadow framebuffer — but only update the depth buffer, skip color entirely.

Store the `light_mvp` matrix in the `ShadowMap` struct — you'll need it in Pass 2.

**Checkpoint:** Write the shadow map's depth buffer to a PPM file as a grayscale image (depth value mapped to brightness). You should see the silhouette of the cube rendered from the light's perspective.

---

### Step 9 — Shadow Testing in Pass 2 (Days 12–13)

In your main render pass, for each pixel being rasterized:

1. Compute the pixel's world-space position (interpolate from world-space triangle vertices using barycentric weights)
2. Transform world position into light clip space using `light_mvp`
3. Apply perspective divide (for orthographic this is trivial — w stays 1)
4. Convert light NDC to shadow map UV: `u = (x_ndc + 1) / 2`, same for v
5. Sample the shadow map at that UV to get `closest_depth`
6. Compare: if `pixel_light_depth > closest_depth + bias` → in shadow

If in shadow: use only ambient lighting
If lit: use full ambient + diffuse + specular

**Bias tuning:** Start with `0.005f`. If you see shadow acne (striped dark patterns on lit surfaces), increase it. If shadows detach from objects, decrease it.

**Checkpoint:** The cube casts a shadow on a flat plane beneath it. The shadow shape matches the cube's silhouette from the light's perspective.

---

### Step 10 — PCF Soft Shadows (Day 14)

Replace the single shadow map sample with a 3×3 grid of samples:

```
shadow_factor = average of 9 samples in a grid around (shadow_u, shadow_v)
```

Each sample returns 0 (lit) or 1 (shadow). The average gives a value in `[0, 1]` that smoothly blends at shadow edges.

Apply the shadow factor to modulate brightness:

```
final_brightness = ambient + (1.0 - shadow_factor) * (diffuse + specular)
```

When `shadow_factor = 1.0` (fully in shadow), only ambient remains.
When `shadow_factor = 0.0` (fully lit), full lighting applies.
When `shadow_factor = 0.5` (edge), you get a smooth blend.

**Checkpoint:** Shadow edges are soft and anti-aliased. No harsh pixel boundaries at the transition between shadow and lit regions.

---

## Concepts Checklist

**Textures**
- [ ] What UV coordinates are and why they're in `[0,1]`
- [ ] Why naive UV interpolation is wrong in perspective
- [ ] What perspective-correct interpolation corrects and why
- [ ] The difference between nearest-neighbor and bilinear sampling
- [ ] What UV wrapping modes do (clamp vs repeat)
- [ ] How lighting modulates texture color

**Shadows**
- [ ] What shadow mapping is conceptually — depth from light's POV
- [ ] Why directional lights use orthographic projection
- [ ] What shadow acne is and why bias fixes it
- [ ] Why too much bias causes peter-panning
- [ ] What PCF does and why it softens shadow edges
- [ ] Why the shadow map needs to be stored after Pass 1

---

## Reading Resources

**Scratchapixel: Texture Mapping**
`scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/texture-coordinates`
The clearest explanation of UV interpolation and perspective correction. Essential reading before Step 4.

**Scratchapixel: Shadow Mapping**
`scratchapixel.com/lessons/3d-basic-rendering/introduction-to-ray-tracing/adding-shadows-to-the-renderer`
Covers the two-pass technique, shadow acne, and bias.

**LearnOpenGL: Shadow Mapping**
`learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping`
Visual diagrams of the depth comparison, acne artifacts, and PCF. Highly recommended alongside Step 9.

**stb_image documentation**
`github.com/nothings/stb/blob/master/stb_image.h`
Read the header comments — everything you need to use it is documented there.

---

## Common Pitfalls

**UV coordinates outside `[0,1]`** — if you don't handle wrapping, out-of-bounds UV access will read garbage memory or crash. Always apply clamping or repeat before converting to pixel coordinates.

**Wrong `clip_w` values** — if you store `clip_w` after the perspective divide instead of before, perspective-correct interpolation will be wrong. Store it from the clip-space vector before calling `perspective_divide`.

**Shadow map resolution too low** — low resolution shadow maps produce blocky shadow edges. If PCF is still producing visible blocks, increase the shadow map resolution.

**Light frustum too large** — if your orthographic projection covers too large an area, the shadow map has low precision. Fit the orthographic frustum tightly around your scene.

**Not clearing the shadow framebuffer** — if you don't reset the shadow map depth buffer to INFINITY before Pass 1, shadows accumulate across frames incorrectly.

**Bias too large** — peter-panning is often worse visually than acne. Start with a small bias and increase only until acne disappears.

---

## Phase 4 Checkpoint

**You are done with Phase 4 when:**

A scene renders with:
- At least one object with a texture mapped correctly — no warping, correct perspective
- Lighting modulating the texture color
- A shadow cast by the object onto another surface
- Soft shadow edges via PCF
- No obvious shadow acne on lit surfaces

And you can answer:
- Why does UV interpolation need perspective correction?
- What does the shadow map store and how is it sampled?
- Why is bias necessary and what does too much bias look like?
- What does PCF do to improve shadow quality?

If you can see a textured, lit, shadowed scene and explain every line that produced it — you have built a complete forward renderer from scratch.