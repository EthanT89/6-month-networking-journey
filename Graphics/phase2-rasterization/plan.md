# Phase 2 — Rasterization: Full Plan

> **Duration:** ~2 weeks of deep work
> **Language:** C
> **Depends on:** Phase 1 math library (vec, mat, camera, projection)
> **Goal:** Take the projected 2D coordinates from Phase 1 and turn them into filled pixels on screen. By the end you will have a renderer that produces solid triangle meshes with correct depth ordering.

---

## What Phase 2 Is

Phase 1 answered: *how does a 3D point become a 2D coordinate?*

Phase 2 answers: *how does a 2D coordinate become pixels?*

Specifically, you are implementing the **rasterizer** — the part of the pipeline that takes projected triangles and determines which pixels they cover, which pixel is in front when triangles overlap, and how to interpolate attributes (color, depth, eventually texture coordinates) across the triangle's surface.

This is the core of real-time rendering. Everything GPUs do at their heart is rasterization.

---

## The Big Picture

After Phase 1 you have screen-space vertices — 2D coordinates with depth values. Phase 2 takes those and does:

```
Screen-space triangle (3 vertices with depth)
        ↓  bounding box
Find candidate pixels
        ↓  barycentric test
Is this pixel inside the triangle?
        ↓  depth test
Is this pixel closer than what's already drawn?
        ↓  interpolation
What color/depth/UV does this pixel have?
        ↓  write to framebuffer
```

Each arrow is a step you will implement.

---

## Concepts to Master

### 1. The Framebuffer

The framebuffer is two things simultaneously:

- A **color buffer** — a 2D array of RGB values, one per pixel. This is what you write to the PPM file.
- A **depth buffer** (z-buffer) — a 2D array of floats, one per pixel. Initialized to the maximum possible depth. Tracks the closest surface seen so far at each pixel.

Every time you consider writing a pixel, you first check the depth buffer. If the new fragment is closer than what's already there, you update both buffers. If it's further, you discard the fragment. This is the **depth test**.

Without the depth buffer, the last triangle drawn always wins — you'd have to sort triangles back-to-front (the painter's algorithm), which fails for intersecting geometry. The depth buffer solves this elegantly with brute force.

**Key insight:** The depth buffer trades memory for correctness. For an 800×600 screen, that's 480,000 floats — about 1.8MB. A small price for correct depth ordering at any complexity.

---

### 2. Bresenham's Line Algorithm

Before rasterizing triangles, you need to rasterize lines. Lines are simpler and serve as the foundation.

The naive approach — compute `y = mx + b` and round to the nearest pixel — has problems. Floating point is slow, and for steep lines you get gaps because you're stepping in X but the line moves faster in Y.

Bresenham's algorithm avoids floating point entirely using only integer arithmetic and a running error accumulator. The core idea: as you step along the major axis (whichever of X or Y changes more), you track an accumulated error. When the error exceeds half a pixel, you step along the minor axis and reset the error.

This algorithm is worth deriving yourself rather than looking up. Think about:
- What does "error" mean geometrically here?
- How do you handle lines in all eight octants (different slopes, different directions)?
- What changes between a shallow line (|dx| > |dy|) and a steep line (|dy| > |dx|)?

**Why it matters beyond lines:** The ideas in Bresenham — stepping along a major axis, accumulating error, snapping to integer coordinates — directly inform how triangle rasterization works. The triangle rasterizer is essentially Bresenham generalized to 2D.

---

### 3. Triangle Rasterization

A triangle is defined by three vertices. The rasterizer needs to find every pixel inside it.

**The naive approach — bounding box + barycentric test:**

1. Find the axis-aligned bounding box of the triangle (min/max of x and y across all three vertices)
2. For every pixel in the bounding box, test whether it's inside the triangle
3. If inside, compute its attributes via interpolation and write to the framebuffer

This is not the most efficient approach, but it's correct, simple to implement, and easy to understand. Real GPU rasterizers use more sophisticated tiling approaches, but the bounding box method is the right starting point.

**The inside test — barycentric coordinates:**

Barycentric coordinates are the key to triangle rasterization. Given a triangle with vertices A, B, C, any point P inside the triangle can be expressed as:

```
P = α*A + β*B + γ*C
```

Where α + β + γ = 1 and all three are non-negative.

Think of α, β, γ as weights — how much each vertex "pulls" on point P. If P is exactly at vertex A, then α=1, β=0, γ=0. If P is at the centroid, all three are 1/3.

The test: compute α, β, γ for a given pixel. If all three are ≥ 0 (and they sum to 1 by construction), the pixel is inside the triangle.

**Why barycentric coordinates are more powerful than just an inside test:**

They're also the interpolation weights. If each vertex has a color, a depth value, a texture coordinate — any attribute — the value at pixel P is exactly:

```
attribute(P) = α * attribute(A) + β * attribute(B) + γ * attribute(C)
```

This is attribute interpolation, and it's how every triangle renderer works. One computation gives you both the inside test *and* the interpolated value. Elegant.

**How to compute barycentric coordinates:**

The most common method uses the signed area of sub-triangles. The signed area of a triangle with vertices (x0,y0), (x1,y1), (x2,y2) is:

```
area = (x1-x0)*(y2-y0) - (x2-x0)*(y1-y0)
```

This is the 2D cross product — it gives a positive value for counterclockwise triangles and negative for clockwise. The barycentric weights are the ratios of the sub-triangle areas to the total triangle area.

---

### 4. Perspective-Correct Interpolation

Here is a subtle but important bug that catches everyone the first time.

When you interpolate attributes using barycentric coordinates in screen space, you get the wrong answer for perspective-projected geometry. The interpolation looks correct for orthographic projection but produces a warped result for perspective.

The reason: barycentric interpolation is linear in screen space, but the original geometry is linear in 3D camera space. The perspective projection is a non-linear transformation — it warps the spacing between points. A point that's halfway between two vertices in screen space is *not* halfway between them in camera space.

The fix is perspective-correct interpolation, which divides the attribute by the depth before interpolating, then divides by the interpolated inverse depth:

```
attribute(P) = (α * attr_A/w_A + β * attr_B/w_B + γ * attr_C/w_C)
             / (α * 1/w_A    + β * 1/w_B    + γ * 1/w_C)
```

Where w_A, w_B, w_C are the clip-space w values (depths) of each vertex.

This feels complex but becomes mechanical once you understand why it's needed. For Phase 2 you'll implement it when interpolating depth values for the z-buffer. It becomes critical in Phase 4 when you interpolate texture coordinates — naïve interpolation produces visibly warped textures on perspective-projected geometry.

---

### 5. Back-Face Culling

For a solid object like a cube, approximately half the triangles face away from the camera at any given time. These back-facing triangles are invisible — they're occluded by the front-facing ones. Rasterizing them is wasted work.

Back-face culling discards these triangles before rasterization. The test is simple: compute the cross product of two triangle edges. If the resulting normal points away from the camera (has a positive z component in screen space for a right-handed system), the triangle is back-facing and gets discarded.

Equivalently: if the signed area of the triangle in screen space is negative, it's back-facing. You're already computing this for barycentric coordinates — back-face culling is essentially free.

**Important:** Back-face culling depends on consistent winding order. If you define your triangles counterclockwise when viewed from the front, all front-facing triangles will have positive signed area and all back-facing ones will be negative. Inconsistent winding produces holes in your mesh.

---

## File Structure

```
phase2-rasterization/
├── README.md
├── log/
└── src/
    ├── main.c               ← test suite and render loop
    ├── framebuffer/
    │   ├── framebuffer.h
    │   └── framebuffer.c    ← color buffer, depth buffer, PPM output
    ├── raster/
    │   ├── raster.h
    │   └── raster.c         ← line drawing, triangle rasterization
    └── geometry/
        ├── geometry.h
        └── geometry.c       ← triangle struct, mesh definition, back-face culling
```

**Shared from Phase 1:**
All of `phase1-math/src/` — vec, mat, camera, projection. Either symlink or copy depending on your build setup. Do not duplicate and modify — keep one source of truth.

---

## Step-by-Step Plan

### Step 1 — Framebuffer (Days 1–2)

Build the infrastructure before the algorithms.

**`framebuffer.h` / `framebuffer.c`:**

```c
struct Framebuffer {
    unsigned char *color;  // width * height * 3 bytes (RGB)
    float *depth;          // width * height floats
    int width;
    int height;
};

struct Framebuffer fb_create(int width, int height);
void fb_clear(struct Framebuffer *fb);
void fb_set_pixel(struct Framebuffer *fb, int x, int y, 
                  unsigned char r, unsigned char g, unsigned char b);
int  fb_depth_test(struct Framebuffer *fb, int x, int y, float depth);
void fb_write_ppm(struct Framebuffer *fb, const char *filename);
void fb_destroy(struct Framebuffer *fb);
```

Key decisions:
- Initialize depth buffer to `INFINITY` (from `<math.h>`) — any real depth will be closer
- `fb_depth_test` should both test AND update the depth buffer if the test passes
- Allocate color and depth buffers dynamically with `malloc` — a 800×600 stack array is pushing it

**Checkpoint:** Clear the framebuffer to a color, draw a few manual pixels, write to PPM, verify it looks right.

---

### Step 2 — Line Drawing (Days 3–4)

Implement Bresenham's line algorithm.

**In `raster.c`:**

```c
void draw_line(struct Framebuffer *fb, 
               int x0, int y0, int x1, int y1,
               unsigned char r, unsigned char g, unsigned char b);
```

Derive it yourself. The key insight is the error accumulator. Think about what "error" means — how far the true line is from the pixel you've chosen — and how to update it each step without division or floating point.

Handle all cases:
- Shallow lines (|dx| > |dy|) — step in X
- Steep lines (|dy| > |dx|) — step in Y
- All four quadrant directions
- Vertical and horizontal edge cases

**Checkpoint:** Draw the 12 edges of a cube wireframe using screen-space coordinates from Phase 1. Should look like a proper wireframe, not dots.

---

### Step 3 — Triangle Definition and Back-Face Culling (Day 5)

**In `geometry.c`:**

```c
struct Triangle {
    struct Vector4 v[3];   // vertices in clip space (with w)
    unsigned char r, g, b; // flat color for now
};

int is_back_face(struct Triangle *t); // returns 1 if back-facing
```

Define a cube as 12 triangles (2 per face, 6 faces). Establish consistent winding order — pick counterclockwise as viewed from outside the cube and stick with it.

**Checkpoint:** Mark back-facing triangles and verify that for a given camera position, roughly half the cube's triangles are culled.

---

### Step 4 — Triangle Rasterization (Days 6–8)

This is the core of Phase 2.

**In `raster.c`:**

```c
void draw_triangle(struct Framebuffer *fb, 
                   struct Vector3 v0, struct Vector3 v1, struct Vector3 v2,
                   unsigned char r, unsigned char g, unsigned char b);
```

Implement in this order:

1. **Bounding box** — find min/max x and y across all three vertices. Clamp to framebuffer bounds.

2. **Barycentric coordinates** — for each pixel in the bounding box, compute the barycentric weights. Use the signed area method.

3. **Inside test** — if all three weights are ≥ 0, the pixel is inside the triangle.

4. **Depth interpolation** — interpolate the depth value using barycentric weights. Use perspective-correct interpolation.

5. **Depth test** — compare interpolated depth against the depth buffer. Only proceed if closer.

6. **Write pixel** — update color buffer and depth buffer.

**Checkpoint:** Render two overlapping triangles of different colors. The correct one should always appear on top regardless of the order you draw them. This verifies the depth buffer is working.

---

### Step 5 — Full Cube Render (Days 9–10)

Wire everything together:

1. Define a cube as 12 triangles with consistent winding
2. Transform each triangle through the MVP pipeline (Phase 1)
3. Back-face cull
4. Clip (simple — skip triangles where any vertex is outside the frustum for now)
5. Rasterize each surviving triangle with a unique color per face
6. Output the PPM

**Checkpoint:** A solid-colored cube rendered correctly — no depth artifacts, back faces hidden, front faces visible with correct depth ordering when faces overlap at the edges.

---

### Step 6 — Vertex Color Interpolation (Days 11–12)

Extend your rasterizer to interpolate per-vertex colors instead of flat triangle colors.

Assign each vertex of the cube a different color. Use barycentric interpolation to smoothly blend between them across the triangle surface. This is the first visual demonstration that your interpolation is working correctly.

**Checkpoint:** A cube with smoothly interpolated colors across each face. The gradient should look smooth with no visible seams at triangle edges.

---

### Step 7 — OBJ Loading (Days 13–14, optional stretch)

Load a real 3D mesh from a `.obj` file instead of a hardcoded cube. The OBJ format is plain text and easy to parse — vertices are lines starting with `v`, faces with `f`.

This extends your renderer to handle arbitrary geometry and sets up Phase 3 (where you'll need real meshes for lighting to look interesting).

---

## Reading Resources

### Core — Read These

**Scratchapixel: Rasterization — A Practical Implementation**
`scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation`
The definitive free resource for exactly what you're building. Covers the full rasterization pipeline including barycentric coordinates, depth buffer, and perspective-correct interpolation. Read alongside your implementation — not before.

**Scratchapixel: The Depth Buffer**
`scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/z-buffer`
Focused specifically on depth buffer mechanics and the z-fighting problem.

**Tiny Renderer by ssloy — Lessons 1 and 2**
`github.com/ssloy/tinyrenderer/wiki`
Now is the right time to look at this. You've built Phase 1 yourself — now read how someone else approached the same problems in Phase 2. Compare their barycentric coordinate implementation to yours. Where do they differ? Why?

### Supplementary

**Real-Time Rendering, 4th Edition — Chapter 23 (Graphics Hardware)**
If you have the book — the section on rasterization gives you the industrial context for what you're implementing by hand.

**3D Math Primer — Chapter on Geometric Tests**
Covers the signed area and barycentric coordinate math with worked examples.

### Watch

**Bisqwit: "Implementing a Software Renderer"** (YouTube)
A live coding video of someone implementing a software renderer. Watching someone else's thought process on the same problem is valuable — especially for debugging intuition.

---

## Concepts Checklist

**Framebuffer**
- [ ] What the color buffer and depth buffer are and why both are needed
- [ ] Why depth buffer is initialized to infinity, not zero
- [ ] What z-fighting is and why it happens

**Line Drawing**
- [ ] How Bresenham's algorithm avoids floating point
- [ ] What the "error accumulator" represents geometrically
- [ ] How to handle all eight octants

**Barycentric Coordinates**
- [ ] How to compute barycentric coordinates from signed areas
- [ ] Why all three weights summing to 1 means the point is on the triangle's plane
- [ ] Why all three being non-negative means the point is inside the triangle
- [ ] How they serve as both an inside test AND interpolation weights

**Depth Testing**
- [ ] Why painter's algorithm fails for intersecting geometry
- [ ] How the depth buffer solves this
- [ ] Why depth values are non-linearly distributed (more precision near camera)

**Perspective-Correct Interpolation**
- [ ] Why naive barycentric interpolation is wrong in perspective
- [ ] What the w-divide correction is and why it works

**Back-Face Culling**
- [ ] How to determine if a triangle is back-facing from signed area
- [ ] What winding order is and why consistency matters

---

## Pitfalls

**Winding order inconsistency** is the most common Phase 2 bug. Define it once, document it, and check every triangle in your mesh. A single flipped triangle produces a hole in your mesh.

**Integer overflow in Bresenham** — if your error accumulator uses `int` and your screen is large, the accumulated error can overflow. Use `long` or be careful about your initial values.

**Off-by-one in bounding box** — when clamping the bounding box to framebuffer bounds, use `max(0, min_x)` and `min(width-1, max_x)`. Forgetting the clamp causes out-of-bounds writes to your pixel buffer.

**Depth buffer precision (z-fighting)** — if near and far are far apart (like 0.1 and 10000), you get very little depth precision at the far end. Coplanar or nearly-coplanar surfaces will flicker. Keep `far/near` ratio as small as practical.

**Forgetting perspective-correct interpolation for depth** — if you use naive barycentric interpolation for the depth value going into the depth buffer, your depth test will be wrong for perspective-projected geometry. The error is subtle and hard to spot visually until you have complex overlapping geometry.

---

## Phase 2 Checkpoint

**You are done with Phase 2 when:**

A solid cube renders correctly with:
- Filled triangle faces (not wireframe)
- Correct depth ordering — no triangles poking through other triangles
- Back faces hidden
- Per-face or per-vertex color
- No depth artifacts on face edges

And you can answer:
- What is barycentric interpolation and why does it work?
- Why is the depth buffer initialized to infinity?
- What breaks without perspective-correct interpolation?
- What is winding order and why does it matter?

If you can see a solid colored cube and explain every line of code that produced it, move to Phase 3 — lighting.