# Phase 4 — Deep Lecture: Textures & Shadow Mapping

> This lecture covers the two major additions in Phase 4 — texture mapping and shadow mapping. By the end of Phase 4, your renderer will map images onto surfaces and cast accurate shadows. These two techniques together produce a result that looks indistinguishable from a basic real-time game engine from the early 2000s.

---

## Part 1 — Texture Mapping

### What a Texture Is

A texture is an image — a 2D array of color values — that gets "painted" onto a 3D surface. Instead of every pixel of a triangle getting a flat color, each pixel samples a color from the texture image at a specific coordinate.

The key question texturing answers: for a given pixel on a triangle, which color from the texture image should it receive?

---

### UV Coordinates

The bridge between 3D geometry and 2D texture images is **UV coordinates** (sometimes called texture coordinates).

Each vertex of a triangle is assigned a 2D coordinate `(u, v)` in the range `[0, 1]`. These coordinates map the vertex to a position in texture space:

- `(0, 0)` — bottom-left of the texture
- `(1, 0)` — bottom-right
- `(0, 1)` — top-left
- `(1, 1)` — top-right

For a cube face, a natural UV assignment wraps the texture across the face:

```
(0,1) ---- (1,1)
  |           |
  |           |
(0,0) ---- (1,0)
```

Each vertex carries a `(u, v)` pair. The rasterizer interpolates `u` and `v` across the triangle using barycentric coordinates — exactly like you interpolated depth and brightness in Phase 2 and 3. At each pixel, the interpolated `(u, v)` tells you which texel (texture pixel) to sample.

---

### Texture Sampling

Given a `(u, v)` coordinate in `[0, 1]`, you need to convert it to actual pixel coordinates in the texture image:

```
tex_x = (int)(u * (texture_width  - 1))
tex_y = (int)(v * (texture_height - 1))
color = texture[tex_y][tex_x]
```

This is **nearest-neighbor sampling** — you just round to the nearest texel. It's fast and produces a sharp, pixelated look (think early Minecraft).

**Bilinear interpolation** is the smoother alternative — instead of snapping to the nearest texel, you sample the four surrounding texels and blend between them based on how close `(u, v)` is to each one. The result is much smoother, especially when the texture is magnified.

```
// For bilinear interpolation:
// Find the four surrounding texels at (x0,y0), (x1,y0), (x0,y1), (x1,y1)
// Blend horizontally, then vertically
// Result is a weighted average of all four
```

Start with nearest-neighbor. Implement bilinear as a stretch goal.

---

### Perspective-Correct UV Interpolation

This is where the Phase 2 concept of perspective-correct interpolation becomes critical.

If you naïvely interpolate `u` and `v` using screen-space barycentric coordinates, the texture will appear warped on perspective-projected surfaces. A checkerboard texture will look like the squares are different sizes depending on depth — closer squares appear larger, further squares smaller, but not in the right way. The warping is most obvious on large triangles at oblique angles.

The fix is the same formula discussed in the Phase 2 lecture:

```
u_correct = (alpha * u0/w0 + beta * u1/w1 + gamma * u2/w2)
          / (alpha * 1/w0  + beta * 1/w1  + gamma * 1/w2)
```

Where `w0`, `w1`, `w2` are the clip-space `w` values of each vertex (the original `-z` values before the perspective divide).

This is why Phase 4 requires you to carry the clip-space `w` values through the pipeline — you need them for correct UV interpolation. You'll add a `clip_w` field to your Triangle struct to store them.

---

### UV Wrapping

What happens when `u` or `v` falls outside `[0, 1]`? You have two choices:

- **Clamp** — values below 0 snap to 0, values above 1 snap to 1. The edge pixel of the texture gets repeated.
- **Repeat** — values wrap around. `u = 1.3` becomes `u = 0.3`. The texture tiles seamlessly across the surface.

Both are useful. Repeat is common for things like brick walls or grass. Clamp is common for decals or unique surface textures.

Implement both — it's just a conditional or a `fmod` operation before the coordinate lookup.

---

### Loading Texture Images

You need to get pixel data from an image file into memory. Writing an image loader from scratch is a significant project in itself — use `stb_image.h` instead.

`stb_image.h` is a single-header C library that loads PNG, JPG, BMP, and other formats into a flat array of bytes. It's one of the most widely used libraries in graphics programming:

```c
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int width, height, channels;
unsigned char *data = stbi_load("texture.png", &width, &height, &channels, 3);
// data is now a flat array: R,G,B,R,G,B,R,G,B,...
// access pixel at (x,y): data[(y * width + x) * 3]
```

The layout is identical to your color buffer — a flat array in row-major order with 3 bytes per pixel. You already know how to work with this format.

---

### The Texture Struct

You'll need a struct to hold texture data:

```c
struct Texture {
    unsigned char *data;
    int width;
    int height;
};
```

With a sampling function:

```c
void texture_sample(struct Texture *tex, float u, float v,
                    unsigned char *r, unsigned char *g, unsigned char *b);
```

And a loader that wraps `stb_image`:

```c
struct Texture texture_load(const char *filename);
void texture_destroy(struct Texture *tex);
```

---

### Applying Textures in the Pipeline

The texture replaces (or modulates) the flat color in `draw_triangle`. Instead of:

```c
fb_set_pixel(fb, x, y, r * brightness, g * brightness, b * brightness);
```

You sample the texture at the interpolated UV coordinate and then apply lighting:

```c
unsigned char tex_r, tex_g, tex_b;
texture_sample(tex, u_interp, v_interp, &tex_r, &tex_g, &tex_b);

fb_set_pixel(fb, x, y,
    (unsigned char)(tex_r * brightness),
    (unsigned char)(tex_g * brightness),
    (unsigned char)(tex_b * brightness));
```

The texture provides the base color, lighting modulates it. They compose cleanly.

---

## Part 2 — Shadow Mapping

### What Shadow Mapping Is

Shadow mapping is a two-pass rendering technique. The insight: a point is in shadow if it's not visible from the light's perspective.

**Pass 1 — Light's perspective:**
Render the entire scene from the light's point of view. Don't write color — just write depth. Store the result in a special texture called the **shadow map** (or depth map).

**Pass 2 — Camera's perspective:**
Render normally from the camera. For each pixel, transform its world-space position into the light's clip space. Compare its depth against the shadow map. If the depth stored in the shadow map is less than the pixel's depth from the light, something else was closer to the light at that point — this pixel is in shadow.

---

### Pass 1 — Building the Shadow Map

The shadow map is just a depth buffer rendered from the light's point of view.

For a directional light, the "camera" is positioned along the light direction, looking toward the scene. You use an **orthographic projection** instead of perspective — a directional light has parallel rays, so there's no perspective foreshortening from the light's point of view.

```
// Light view matrix — position the "camera" along the light direction
light_view = lookAt(light_pos, scene_center, up)

// Orthographic projection — no perspective
light_proj = orthographic(left, right, bottom, top, near, far)

// Light space transform
light_mvp = light_proj * light_view * model
```

Render every triangle through `light_mvp`, updating a depth buffer but skipping the color buffer entirely. This depth buffer IS your shadow map.

---

### Pass 2 — Using the Shadow Map

For each pixel during the normal camera render, you need to know its position in light space.

You have the pixel's world-space position (from the world_verts you already compute for lighting). Transform it into light clip space:

```
light_clip_pos = light_mvp * world_pos
```

After the perspective divide (for directional light with orthographic projection, w=1 so this is trivial), you have the pixel's position in light NDC space. Convert to shadow map coordinates:

```
shadow_u = (light_ndc.x + 1) / 2
shadow_v = (light_ndc.y + 1) / 2
```

Sample the shadow map at `(shadow_u, shadow_v)` to get the closest depth the light saw at this position. Compare against the pixel's depth in light space:

```
if (pixel_depth_in_light_space > shadow_map_depth + bias):
    // in shadow — only use ambient lighting
else:
    // lit — use full lighting
```

---

### Shadow Acne

Shadow acne is a numerical precision artifact that appears as a striped or mottled pattern on lit surfaces. It happens because the depth comparison is made between a value stored at limited floating-point precision and a freshly computed depth — rounding errors cause some pixels to incorrectly shadow themselves.

The fix is a **bias** — a small constant added to the shadow map depth (or subtracted from the pixel's depth) before comparison:

```
if (pixel_light_depth > shadow_depth + bias):
    in_shadow = 1
```

Typical bias values are `0.005` to `0.05`. Too small and acne persists. Too large and shadows "detach" from their casters — a shadow appears to float away from the object's base. Finding the right bias for your scene requires tuning.

---

### Percentage Closer Filtering (PCF)

Hard shadow edges look harsh and aliased. **PCF** softens shadow edges by sampling the shadow map multiple times at nearby coordinates and averaging the results:

```
// Sample a 3x3 grid around the shadow coordinate
float shadow = 0;
for (dx = -1; dx <= 1; dx++) {
    for (dy = -1; dy <= 1; dy++) {
        float closest = sample_shadow_map(u + dx*texel_size, v + dy*texel_size);
        shadow += (pixel_depth > closest + bias) ? 1.0 : 0.0;
    }
}
shadow /= 9.0;  // average — gives soft edges
```

PCF gives you penumbra-like soft shadows without ray tracing. It's the standard technique in real-time rendering.

---

### The Shadow Map as a Texture

Your shadow map is implemented as a `Framebuffer` where you only use the depth buffer. After Pass 1, the depth buffer of your shadow framebuffer IS the shadow map.

You'll need a way to sample the depth buffer at UV coordinates — essentially treating it like a texture. This is one of the few places where your framebuffer infrastructure from Phase 2 gets extended rather than reused as-is.

---

## Part 3 — Putting It Together

### The Full Phase 4 Pipeline

```
// Pass 1 — Shadow map
for each triangle:
    transform through light_mvp
    rasterize into shadow_framebuffer (depth only)

// Pass 2 — Final render
for each triangle:
    transform through camera_mvp → screen space
    compute lighting:
        - face normal (from world space, Phase 3)
        - diffuse + specular (from Phase 3)
        - shadow test (new — is this triangle in shadow?)
        - if in shadow: brightness = ambient only
        - if lit: brightness = ambient + diffuse + specular
    rasterize:
        - interpolate UV coordinates (perspective-correct)
        - sample texture at UV
        - apply brightness to texture color
        - depth test
        - write pixel
```

---

## Part 4 — Key Concepts to Internalize

### Why Two Passes?

The shadow map approach trades one extra render pass for correct shadows at any scene complexity. Alternative approaches — shadow volumes, ray tracing — are more accurate but more complex or expensive. Shadow mapping is the dominant technique in real-time rendering because the cost (one extra depth render) is predictable and scalable.

### Why Orthographic for Directional Lights?

A directional light has parallel rays — no point of origin, no perspective. Orthographic projection preserves parallel lines, which matches the physical behavior of a directional light (like the sun). Using perspective projection for a directional light would introduce foreshortening that doesn't exist in the real light.

Point lights use perspective projection for shadow maps (or cube maps — six shadow maps, one per face of a cube).

### Why Does UV Need Perspective Correction?

Screen-space barycentric coordinates are in a warped space — the perspective projection non-linearly shrinks distant geometry. Linear interpolation in this warped space produces linear results in screen space, not in the original 3D space where the texture lives. The perspective correction formula "undoes" the warp before interpolating, giving you correct linear interpolation in 3D space projected back to screen.

### The Bias-Acne Tradeoff

Shadow bias is one of the most commonly tuned parameters in real-time renderers. Too little = acne. Too much = Peter-panning (shadows detach from objects). The right value depends on scene scale, light angle, and shadow map resolution. Knowing this tradeoff exists — and why — is more important than knowing the exact value.

---

## What Phase 4 Produces

A scene with:
- Textured surfaces — images mapped correctly onto geometry with perspective correction
- Cast shadows — objects blocking light produce accurate shadow regions on other surfaces
- Full Phong lighting modulating the texture color
- Soft shadow edges via PCF

This is the complete pipeline of a basic forward renderer — the same architecture used in games from roughly 2000-2010 before deferred rendering became dominant.