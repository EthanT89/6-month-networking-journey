# Phase 3 — Lighting: Full Plan

> **Duration:** ~2 weeks of deep work
> **Language:** C
> **Depends on:** Phase 1 math library, Phase 2 rasterizer and framebuffer
> **Goal:** Implement physically grounded lighting — surface normals, the Phong model, multiple light sources. Your cube goes from flat colored polygons to a shaded 3D object that reacts to light.

---

## What Phase 3 Is

Phase 2 gave you filled triangles. Phase 3 gives those triangles a relationship with light.

Right now every pixel of a face gets the same flat color regardless of where the light is or which direction the face points. Phase 3 changes that. By the end, your renderer will compute how bright each triangle face is based on:

- The direction the face points (its normal vector)
- The direction and color of one or more light sources
- The position of the camera (for specular highlights)

The result looks like a real object under real lighting — bright where light hits directly, dark where it faces away, with a specular highlight where the light reflects toward your eye.

---

## The Big Picture

The Phong lighting model has three components that you combine:

```
final_color = ambient + diffuse + specular
```

Each component answers a different physical question:

- **Ambient** — how much light reaches this surface even with no direct illumination? (approximates indirect/bounced light)
- **Diffuse** — how much light hits this surface directly? (depends on the angle between the surface normal and the light direction)
- **Specular** — how much light reflects directly toward the camera? (depends on the angle between the reflected light direction and the view direction)

You implement them in this order — ambient first (trivial), diffuse second (the most important), specular third (the polish). Each one builds on the previous.

---

## Concepts to Master

### 1. Surface Normals

A normal is a vector perpendicular to a surface. For a triangle, the normal is perpendicular to the plane the triangle lies in.

**Computing a triangle normal:**

Given three vertices A, B, C:

```
edge1 = B - A
edge2 = C - A
normal = normalize(cross(edge1, edge2))
```

The cross product of two edges gives a vector perpendicular to both — perpendicular to the triangle's plane. Normalizing makes it unit length, which is required for the dot product lighting math to work correctly.

**Winding order matters here too.** The direction of the cross product depends on the order of the edges. With counterclockwise winding (which you've already established), `cross(edge1, edge2)` points toward the viewer for front-facing triangles. Consistent winding order means consistent normals pointing outward.

**Flat vs smooth shading:**

- **Flat shading** — one normal per triangle, computed from its edges. All pixels in the triangle get the same brightness. This is what you'll implement first — it's what gives the distinctive faceted look.
- **Smooth shading (Gouraud)** — one normal per vertex, averaged from all triangles sharing that vertex. Each pixel interpolates between vertex normals using barycentric coordinates. This produces smooth curved-looking surfaces.

Start with flat shading. You already have the tools to do it — cross product and normalize are both in your vec library.

**Critical detail — normal transform:**

When you apply a model matrix to your vertices, the normals do NOT transform the same way. You cannot multiply a normal by the model matrix. The correct transform for normals is the **transpose of the inverse** of the model matrix's upper 3×3.

For rotation-only model matrices (no non-uniform scale), the model matrix itself works fine for normals — rotation matrices are orthogonal so their transpose equals their inverse, and the transpose of the inverse is just the matrix itself.

For now, compute normals in world space after applying only the rotation part of the model matrix. When you add non-uniform scaling later, you'll need the full normal matrix.

---

### 2. Diffuse Lighting (Lambertian Reflectance)

This is the most important lighting component — it's what makes a surface look 3D.

The physical model: a surface reflects more light when it faces directly toward the light source, and less when it's angled away. A surface facing exactly away from the light reflects nothing.

**The math:**

```
diffuse = max(0, dot(normal, light_dir)) * light_color * surface_color
```

Where:
- `normal` is the unit surface normal
- `light_dir` is the unit vector pointing FROM the surface TOWARD the light
- `max(0, ...)` clamps negative values — a surface facing away gets zero light, not negative

**Why `dot(normal, light_dir)` works:**

The dot product of two unit vectors equals `cos(θ)` where θ is the angle between them. When the normal points directly at the light (θ=0°), `cos(0)=1` — full brightness. When the normal is perpendicular to the light (θ=90°), `cos(90)=0` — no contribution. When facing away (θ>90°), the cosine is negative — clamped to zero.

This is exactly how real surfaces behave — it's the physical law of how light scatters off matte surfaces (Lambertian reflectance). One dot product, one clamp, and you have physically correct diffuse lighting.

**The light direction:**

For a directional light (like the sun), the light direction is the same everywhere — a single unit vector. This is the simplest case:

```
light_dir = normalize(light_position - surface_point)  // point light
light_dir = normalize(-light_direction)                 // directional light
```

Note: `light_dir` points FROM the surface TOWARD the light — not the direction the light travels. Getting this direction backwards is the most common bug in diffuse lighting.

---

### 3. Ambient Lighting

The simplest component — a constant base brightness added to everything:

```
ambient = ambient_strength * light_color * surface_color
```

`ambient_strength` is typically a small value like `0.1` — just enough so surfaces facing away from the light aren't pure black. In reality, ambient light comes from indirect bounced light — walls, ceiling, other surfaces. The Phong model approximates all of that with a single constant.

Without ambient, back-facing surfaces are completely black, which looks harsh and unrealistic. With ambient, they have a small base brightness that feels natural.

---

### 4. Specular Highlights

Specular highlights are the bright spots you see on shiny surfaces — where the light reflects almost directly into your eye. Think of a polished apple or a metal sphere.

**The math:**

```
reflect_dir = reflect(-light_dir, normal)
spec_strength = pow(max(0, dot(reflect_dir, view_dir)), shininess)
specular = spec_strength * light_color * specular_color
```

Where:
- `reflect_dir` is the direction of the reflected light ray
- `view_dir` is the unit vector pointing FROM the surface TOWARD the camera
- `shininess` controls how tight/sharp the highlight is — low values (8–16) give broad matte highlights, high values (64–256) give tight mirror-like highlights
- `pow(..., shininess)` makes the highlight fall off rapidly away from the perfect reflection angle

**The reflect function:**

```
reflect(incident, normal) = incident - 2 * dot(incident, normal) * normal
```

Where `incident` is the incoming light direction (pointing toward the surface). This is the standard reflection formula — the reflected ray is the incident ray mirrored across the normal.

**Blinn-Phong optimization:**

Instead of computing the reflection vector, Blinn-Phong uses the halfway vector between the light direction and view direction:

```
halfway = normalize(light_dir + view_dir)
spec_strength = pow(max(0, dot(normal, halfway)), shininess)
```

This is computationally cheaper and actually matches real surface behavior more accurately at grazing angles. Most real-time renderers use Blinn-Phong rather than Phong. Implement Blinn-Phong.

---

### 5. Combining the Components

The full Phong/Blinn-Phong lighting calculation per pixel:

```c
// Inputs (all unit vectors):
// normal     — surface normal
// light_dir  — direction toward the light
// view_dir   — direction toward the camera
// light_color
// surface_color
// ambient_strength (e.g. 0.1)
// shininess (e.g. 32)

vec3 ambient  = ambient_strength * light_color * surface_color;

float diff    = max(0, dot(normal, light_dir));
vec3 diffuse  = diff * light_color * surface_color;

vec3 halfway  = normalize(light_dir + view_dir);
float spec    = pow(max(0, dot(normal, halfway)), shininess);
vec3 specular = spec * light_color * specular_color;

vec3 result   = ambient + diffuse + specular;
```

The result is clamped to [0, 255] per channel before writing to the framebuffer.

---

### 6. Point Light vs Directional Light

**Directional light** — infinitely far away, parallel rays. The light direction is the same for every surface point. Think: the sun.

```c
struct DirectionalLight {
    struct Vector3 direction;  // unit vector, points TOWARD the light
    float r, g, b;             // color and intensity
};
```

**Point light** — a light at a specific position in space. The direction varies per surface point — you compute it as `normalize(light_pos - surface_pos)`. Intensity falls off with distance (attenuation).

```c
struct PointLight {
    struct Vector3 position;
    float r, g, b;
    float constant;    // attenuation constants
    float linear;
    float quadratic;
};

// Attenuation
float dist  = length(light_pos - surface_pos);
float atten = 1.0 / (constant + linear*dist + quadratic*dist*dist);
```

Start with a directional light — simpler, no attenuation needed. Add point lights after diffuse lighting is working.

---

### 7. Where Lighting Computation Happens

For flat shading, lighting is computed **once per triangle** using the triangle's face normal. The result is a single color applied uniformly to every pixel in the triangle.

For smooth shading (Gouraud), lighting is computed **once per vertex** using per-vertex normals. The resulting colors are then interpolated across the triangle using barycentric coordinates — which you've already implemented.

For per-pixel lighting (Phong shading), normals are interpolated across the triangle and lighting is computed **once per pixel**. This produces the smoothest results but is the most expensive.

Implement flat shading first. Smooth shading (Gouraud) is a natural extension — you already have barycentric interpolation.

---

## File Structure

```
phase3-lighting/
├── README.md
├── log/
└── src/
    ├── main.c                    ← test suite and render loop
    ├── lighting/
    │   ├── lighting.h
    │   └── lighting.c            ← normal computation, Phong model, light structs
    └── geometry/
        ├── geometry.h
        └── geometry.c            ← extend Phase 2 geometry with normals
```

**Shared from Phase 1 and 2:**
- `phase1-math/src/` — vec, mat, camera, projection
- `phase2-rasterization/src/framebuffer/` — framebuffer
- `phase2-rasterization/src/raster/` — draw_triangle, draw_line
- `phase2-rasterization/src/geometry/` — Triangle struct, is_back_face

Extend the `Triangle` struct to include a normal vector and lighting parameters. Don't duplicate — extend.

---

## Step-by-Step Plan

### Step 1 — Surface Normal Computation (Days 1–2)

**In `lighting.c`:**

```c
struct Vector3 compute_face_normal(struct Vector3 v0, struct Vector3 v1, struct Vector3 v2);
```

Implement using cross product of two edges. Normalize the result.

Extend your `Triangle` struct to carry a precomputed normal:

```c
struct Triangle {
    struct Vector3 v[3];
    struct Vector3 normal;     // face normal in world space
    unsigned char r, g, b;
};
```

After building your 12 cube triangles, compute and store the normal for each one. Verify by printing normals — the front face normal should point in the +Z direction (or the direction facing the camera), the top face should point in +Y, etc.

**Checkpoint:** Print all 6 face normals. They should be axis-aligned for an unrotated cube — `(0,0,1)`, `(0,0,-1)`, `(1,0,0)`, `(-1,0,0)`, `(0,1,0)`, `(0,-1,0)`.

---

### Step 2 — Ambient Lighting (Day 3)

The simplest possible lighting — modify `draw_triangle` to accept a brightness multiplier and apply it to the color:

```c
void draw_triangle(struct Framebuffer *fb,
                   struct Vector3 v0, struct Vector3 v1, struct Vector3 v2,
                   float brightness,
                   unsigned char r, unsigned char g, unsigned char b);
```

For ambient only, `brightness = ambient_strength` (e.g. `0.1`).

Apply it when writing the pixel:

```c
fb_set_pixel(fb, x, y,
    (unsigned char)(r * brightness),
    (unsigned char)(g * brightness),
    (unsigned char)(b * brightness));
```

**Checkpoint:** Render the cube with `brightness = 0.1`. Everything should be dim but visible. Then try `brightness = 1.0` — back to full white. The multiplier works.

---

### Step 3 — Diffuse Lighting (Days 4–6)

Add a directional light struct:

```c
struct DirectionalLight {
    struct Vector3 direction;  // unit vector pointing toward the light
    float intensity;
    unsigned char r, g, b;
};
```

Implement the lighting calculation:

```c
float compute_diffuse(struct Vector3 normal, struct DirectionalLight light);
```

```
diffuse = max(0, dot(normal, light.direction)) * light.intensity
```

For each triangle:
1. Get the face normal (already stored)
2. Compute diffuse factor
3. Add ambient: `brightness = ambient + diffuse`
4. Clamp to [0, 1]
5. Pass to `draw_triangle`

**Checkpoint:** A cube where visible faces are lit based on their orientation to the light. The face pointing most directly at the light should be brightest. Faces perpendicular to the light should be dim (ambient only). Back faces are culled so you won't see them, but if you temporarily disable culling you should see them at ambient brightness.

Try rotating the light direction and watch the shading change.

---

### Step 4 — Normal Transform (Day 7)

When your cube has rotation applied, the precomputed normals need to be transformed too.

For a rotation-only model matrix, multiply the normal by the rotation matrix:

```c
struct Vector3 transform_normal(struct Vector3 normal, struct Matrix model);
```

Since your rotation matrix is orthogonal, this is just a `mat4_mul_vec4` on the normal (with `w=0` since it's a direction, not a point), then extract xyz.

After building the model matrix and before computing lighting, transform each triangle's normal. Verify that a rotated cube still has normals pointing in the right directions — the lit face should visually match the face that geometrically faces the light.

**Checkpoint:** Rotate the cube 45 degrees and verify the shading is geometrically correct — the diagonal face catching the most light should appear brightest.

---

### Step 5 — Specular Highlights (Days 8–10)

Add the camera position to your lighting setup — you need the view direction per triangle:

```c
struct Vector3 view_dir = normalize(camera_pos - triangle_center);
```

Where `triangle_center` is the average of the three vertex positions in world space.

Implement Blinn-Phong specular:

```c
float compute_specular(struct Vector3 normal, struct Vector3 light_dir,
                       struct Vector3 view_dir, float shininess);
```

```
halfway = normalize(light_dir + view_dir)
specular = pow(max(0, dot(normal, halfway)), shininess)
```

Combine all three components:

```c
float brightness = ambient + diffuse + specular;
brightness = fminf(brightness, 1.0f);  // clamp
```

**Checkpoint:** A cube with a visible specular highlight — a bright spot on the face most directly reflecting light toward the camera. Try different shininess values (8, 32, 128) and observe how the highlight changes from broad to tight.

---

### Step 6 — Multiple Lights (Days 11–12)

Support an array of directional lights. The lighting calculation is additive — sum the diffuse and specular contributions from each light, then add ambient once:

```c
float brightness = ambient;
for (int i = 0; i < num_lights; i++) {
    brightness += compute_diffuse(normal, lights[i]);
    brightness += compute_specular(normal, lights[i].direction, view_dir, shininess);
}
brightness = fminf(brightness, 1.0f);
```

Add a second light from a different direction and observe how the cube responds.

**Checkpoint:** A cube lit by two directional lights from different angles. Faces visible to both lights should be brighter than faces visible to only one.

---

### Step 7 — Smooth Shading / Gouraud (Days 13–14, stretch)

Instead of a flat color per triangle, compute lighting at each vertex using per-vertex normals, then interpolate the resulting colors across the triangle using your existing barycentric interpolation.

Per-vertex normals for a cube are the average of all face normals sharing that vertex. For a cube this is a 45-degree diagonal for corner vertices, which produces a smooth rounded appearance even though the geometry is flat.

Modify `draw_triangle` to accept per-vertex brightness values and interpolate them:

```c
void draw_triangle_gouraud(struct Framebuffer *fb,
                            struct Vector3 v0, struct Vector3 v1, struct Vector3 v2,
                            float b0, float b1, float b2,
                            unsigned char r, unsigned char g, unsigned char b);
```

Inside the rasterization loop, interpolate brightness:

```c
float pixel_brightness = alpha * b0 + beta * b1 + gamma * b2;
```

**Checkpoint:** A cube that looks rounder and smoother even though it's the same geometry. The harsh triangle seams from flat shading disappear.

---

## Reading Resources

### Core — Read These

**Scratchapixel: Introduction to Shading**
`scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading`
The best free resource on this exact topic. Covers normals, diffuse, specular, and the math behind each. Read alongside your implementation.

**LearnOpenGL: Basic Lighting**
`learnopengl.com/Lighting/Basic-Lighting`
Extremely clear walkthrough of the Phong model with visual diagrams for each component. The diagrams of ambient/diffuse/specular contributions are worth bookmarking.

**LearnOpenGL: Materials**
`learnopengl.com/Lighting/Materials`
Covers material properties — different `ambient`, `diffuse`, `specular`, and `shininess` values for different surface types (gold, rubber, obsidian, etc.). Good for understanding how the model parameters map to real-world materials.

### Supplementary

**Mathematics for 3D Game Programming — Chapter 7: Illumination**
If you have the Lengyel book, this chapter derives the Phong model rigorously and covers both Phong and Blinn-Phong with full mathematical justification.

**Real-Time Rendering — Chapter 5: Shading Basics**
The definitive reference. Dense but complete — covers everything from flat shading through physically-based rendering. Use as a reference, not a tutorial.

### Watch

**3Blue1Brown: Dot Products and Duality**
`youtube.com/3blue1brown`
If the geometric meaning of `dot(normal, light_dir)` = brightness isn't fully intuitive yet, this video will lock it in permanently. The dot product as a projection operation is exactly what's happening in diffuse lighting.

---

## Concepts Checklist

**Normals**
- [ ] How to compute a face normal from three vertices
- [ ] Why the cross product gives a perpendicular vector
- [ ] Why winding order determines which way the normal points
- [ ] Why normals transform differently from positions (transpose of inverse)
- [ ] Difference between flat shading (per-face normal) and smooth shading (per-vertex normal)

**Diffuse lighting**
- [ ] Why `dot(normal, light_dir)` gives the diffuse contribution
- [ ] Why you clamp to max(0, ...) — no negative light
- [ ] The difference between directional light and point light
- [ ] Why light_dir points TOWARD the light, not away from it

**Ambient lighting**
- [ ] What ambient approximates physically (indirect/bounced light)
- [ ] Why it's a constant — not dependent on normal or light direction

**Specular highlights**
- [ ] What the reflect function computes geometrically
- [ ] Why shininess controls highlight size
- [ ] Difference between Phong (reflection vector) and Blinn-Phong (halfway vector)
- [ ] Why view direction matters for specular but not diffuse

**Combining components**
- [ ] Why ambient is added once, not per-light
- [ ] Why you clamp the final brightness to [0, 1]
- [ ] How multiple lights combine (additive)

---

## Common Pitfalls

**Light direction pointing the wrong way** — the most common bug. `light_dir` must point FROM the surface TOWARD the light. If you get it backwards, surfaces facing the light appear dark and surfaces facing away appear bright.

**Forgetting to normalize** — the dot product only equals `cos(θ)` when both vectors are unit length. Unnormalized normals or light directions produce incorrect brightness values. Normalize after every operation that could change the length.

**Not transforming normals when the model rotates** — if you compute normals in model space and apply a rotation, the normals won't match the actual triangle orientation. Transform normals by the rotation matrix whenever you transform vertices.

**Ambient applied per-light** — ambient should be added once, not multiplied by the number of lights. Adding it per-light makes the scene uniformly brighter with each light added, which is physically wrong.

**Specular on back faces** — if you compute specular for triangles where `dot(normal, light_dir) < 0` (facing away from the light), you get specular highlights on surfaces that aren't even lit. Guard specular computation with `if (diff > 0)`.

**Integer overflow in color math** — if you compute `r * brightness` and `r` is 255 and `brightness` is 1.0, the result is 255 — fine. But if you do color math in `unsigned char` space before clamping, you can overflow. Do all lighting math in `float`, then convert to `unsigned char` at the end.

---

## Phase 3 Checkpoint

**You are done with Phase 3 when:**

A solid cube renders with the full Phong lighting model:
- Ambient base brightness — no face is pure black
- Diffuse shading — faces directly lit are bright, faces angled away are dim
- Specular highlight — a visible bright spot where light reflects toward the camera
- The shading updates correctly when you change the light direction or rotate the cube
- Multiple lights combine additively

And you can answer:
- Why does `dot(normal, light_dir)` give you brightness?
- What does the shininess exponent control geometrically?
- Why do normals transform differently from vertex positions?
- What is the difference between Phong and Blinn-Phong?

If you can see physically plausible shading on your cube and explain every calculation that produced it, move to Phase 4 — textures and shadows.