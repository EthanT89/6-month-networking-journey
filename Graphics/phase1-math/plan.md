# Phase 1 — Math Foundation: Deep Plan

> **Duration:** ~2 weeks of deep work (flexible — go deeper if needed)
> **Language:** C
> **Goal:** Build a complete linear algebra library from scratch. Understand every operation geometrically, not just mechanically.

---

## The mindset for this phase

Phase 1 is deceptively important. Most people treat it as a prerequisite — a box to check before the "real" work. Don't. The engineers who truly understand coordinate spaces and matrix math can *reason* about graphics problems intuitively. Those who skipped the depth spend years debugging transforms they don't fully understand.

Every concept here has a geometric meaning. Always ask: *what is this operation doing to space?*

---

## Concepts to Master

### 1. Vectors — the atoms of 3D math

**What they are:** A direction and a magnitude in space. Not a position — a *relationship*.

| Operation | What it does geometrically |
|---|---|
| Addition | Walk one vector, then walk another |
| Scalar multiply | Stretch or shrink a vector |
| Dot product | Measures how much two vectors point in the same direction. Result is a scalar. |
| Cross product | Returns a vector perpendicular to both inputs. Magnitude = area of the parallelogram they form. |
| Normalize | Scale to length 1 — pure direction, no magnitude |
| Magnitude | Length of the vector via Pythagorean theorem |

**Deep insight to internalize:** The dot product `dot(A, B) = |A||B|cos(θ)`. When both vectors are unit length, `dot(A, B) = cos(θ)`. This is how you check if a surface faces toward a light — just dot the surface normal with the light direction. If the result is positive, it faces the light. If negative, it faces away. This single idea drives all diffuse lighting.

**Things to implement:**
- `vec3` struct: x, y, z
- `vec4` struct: x, y, z, w (needed for matrix multiplication)
- All operations above for both types
- Test every function with known values you can verify by hand

---

### 2. Matrices — transformations as objects

**What they are:** A matrix is a *linear transformation* — a function that takes a vector in, applies some operation (rotate, scale, translate), and gives a transformed vector out. The magic: you can compose multiple transforms into one matrix by multiplying them.

| Matrix | What it does |
|---|---|
| Identity | Does nothing — `M * v = v` |
| Scale | Stretches space along axes |
| Rotation (X/Y/Z) | Rotates space around an axis |
| Translation | Moves points (requires 4×4 homogeneous) |
| Transpose | Flips rows and columns — useful for inverting orthogonal matrices |

**Deep insight to internalize:** Why 4×4 instead of 3×3? Because translation is *not* a linear operation — you can't encode it in a 3×3 matrix. By adding a 4th coordinate `w=1` for points and `w=0` for vectors, you can express translation as matrix multiplication. This is the homogeneous coordinate trick and it's fundamental to everything.

**Things to implement:**
- `mat4` struct: `float m[4][4]`
- Identity matrix constructor
- Matrix multiplication (`mat4_mul`)
- Matrix-vector multiplication (`mat4_mul_vec4`)
- Transpose
- Translation, rotation (X, Y, Z), scale constructors

**Critical detail:** Row-major vs column-major. Pick one and be consistent. C naturally stores 2D arrays in row-major order. OpenGL uses column-major. Know which one you're using — it affects how you pass data to shaders later.

---

### 3. Coordinate Spaces — the hierarchy of "where am I?"

This is the concept most graphics resources explain poorly. Sit with it.

```
Object Space → World Space → Camera Space → Clip Space → NDC → Screen Space
```

| Space | Description |
|---|---|
| Object/Model space | The object's local origin. A cube centered at (0,0,0). |
| World space | Where things live in the scene. The cube is at (5, 0, 3). |
| Camera/View space | The world as seen from the camera's perspective. Camera is always at the origin, pointing down -Z. |
| Clip space | After projection matrix is applied. Used for clipping geometry. |
| NDC (Normalized Device Coordinates) | After perspective divide. All visible geometry lives in [-1, 1] on all axes. |
| Screen/Raster space | NDC mapped to pixel coordinates. (0,0) is usually top-left. |

**Deep insight:** The view matrix doesn't move the camera — it moves the *entire world* so the camera ends up at the origin. Same result, different frame of reference. This is why `view = inverse(camera_transform)`.

**Things to implement:**
- `lookAt(eye, center, up)` → returns a `mat4` view matrix
- `perspective(fov, aspect, near, far)` → returns a `mat4` projection matrix
- `orthographic(left, right, bottom, top, near, far)` → returns a `mat4`
- NDC → screen space mapping function

---

### 4. The Transform Chain

The full pipeline from object to pixel:

```
clip_pos = projection * view * model * local_pos
```

This single line is the spine of all real-time rendering. Understand every matrix in it deeply.

- **Model matrix:** Places the object in the world (translate + rotate + scale)
- **View matrix:** Converts world space to camera space
- **Projection matrix:** Applies perspective (makes far things small)
- **Perspective divide:** Divide by `w` to get NDC

**Things to implement:**
- A function that composes model + view + projection into one MVP matrix
- Apply it to a set of cube vertices
- Map resulting NDC coordinates to screen pixels
- Draw the projected vertices as dots (validation checkpoint)

---

### 5. Gimbal Lock & Quaternions (optional stretch goal)

Rotation matrices work fine for Phase 1. But know that Euler angles (rotating around X, then Y, then Z) have a nasty problem called *gimbal lock* — when two rotation axes align, you lose a degree of freedom. Quaternions solve this. You won't need them for the renderer but understanding the problem is valuable. Bookmark for later.

---

## Week-by-Week Breakdown

### Week 1 — Vectors, Matrices, Transforms

**Days 1–2:** Implement `vec3`, `vec4`. Write a test file. Verify dot product, cross product, normalize by hand.

**Days 3–4:** Implement `mat4`. Get matrix multiply correct — this is where most bugs hide. Test: `identity * M = M`.

**Day 5:** Implement translation, rotation (X/Y/Z), scale matrices. Compose them. Apply to a point.

**Day 6–7:** Write a test that transforms a cube's 8 vertices through a model matrix. Log the results. Verify a few by hand.

---

### Week 2 — Coordinate Spaces & Projection

**Days 1–2:** Implement `lookAt`. Understand it geometrically: it's building a coordinate frame from three vectors (eye, center, up). Test: camera at (0,0,5) looking at origin should place origin at (0,0,-5) in view space.

**Days 3–4:** Implement perspective projection matrix. This is the hardest derivation in Phase 1. Don't just copy the formula — derive it from first principles using similar triangles. Scratchapixel's lesson on this is the best explanation available.

**Day 5:** Implement the NDC → screen space mapping. Chain the full MVP pipeline.

**Days 6–7:** Final checkpoint: project all 8 vertices of a cube onto a 2D canvas. Write them to a simple PPM file (no SDL needed yet — PPM is just a text file of RGB values). The cube's wireframe should look correct.

---

## Reading Order & Resources

Work through these in order. Don't skip ahead.

### Free (Start Here)

**Scratchapixel 2.0** — `scratchapixel.com`
The single best free resource for this phase. Read these lessons in order:

1. [Geometry](https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/points-vectors-and-normals.html) — points, vectors, normals, dot/cross products, matrices, coordinate systems
2. [Computing Pixel Coordinates of a 3D Point](https://www.scratchapixel.com/lessons/3d-basic-rendering/computing-pixel-coordinates-of-3d-point/perspective-projection.html) — the full world→camera→screen pipeline
3. [The Perspective and Orthographic Projection Matrix](https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/projection-matrices-what-you-need-to-know-first.html) — how to derive and construct the projection matrix

**3Blue1Brown — Essence of Linear Algebra** (YouTube)
Watch before or alongside coding. It builds the geometric intuition that transforms abstract matrix operations into things you can *see*. Especially chapters on linear transformations, matrix multiplication, and the determinant. Free.

**Khan Academy — Linear Algebra**
Good supplement for filling gaps. The dot product and cross product sections are particularly clear.

---

### Books to Order

These are ranked by priority for Phase 1 specifically.

**Priority 1 — Order Now**

**3D Math Primer for Graphics and Game Development** — Fletcher Dunn & Ian Parberry
~$50, available on Amazon. This is the most approachable book specifically written for programmers learning graphics math. It covers vectors, matrices, coordinate spaces, and transformations with visual explanations and worked examples. Written exactly for someone at your stage. Widely regarded as one of the clearest introductions to 3D math for game and graphics development. Start reading this in Week 1 alongside your coding.

**Priority 2 — Order Soon (useful in Phase 3+)**

**Mathematics for 3D Game Programming and Computer Graphics, 3rd Edition** — Eric Lengyel
~$60, Amazon. Starts at a fairly basic level in vectors and linear algebra, then progresses to more advanced topics including illumination and visibility. Particular attention is given to derivations of key results — the reader is not left with gaps in the theory. This is the more rigorous, complete reference. Use it alongside Phase 1 for deeper derivations, and heavily in Phases 3 and 4 for lighting and shadows. Lengyel has a PhD in CS and MS in Mathematics — the depth shows.

**Priority 3 — Optional / Later**

**Real-Time Rendering, 4th Edition** — Akenine-Möller et al.
~$90. The definitive industry reference. Dense. Not a teaching book — more of an encyclopedia you'll consult repeatedly across all phases. If you only get one "expensive" book for the whole journey, make it this one.

---

### Online Tools

**GeoGebra 3D** — `geogebra.org/3d`
Free, browser-based. Visualize vectors, dot products, cross products, coordinate transformations interactively. Invaluable for building geometric intuition. When you're confused about why a rotation matrix looks the way it does, plot it here.

**Desmos** — `desmos.com`
For visualizing 2D projections and understanding how the perspective divide maps 3D points to 2D.

**Shadertoy** — `shadertoy.com`
Look at simple shaders to see how the math you're implementing gets used in real code. Not for Phase 1 specifically, but good to have open as context.

---

## Concepts Checklist

Use this to track mastery. "Know it" means you can derive it from memory and explain the geometric meaning.

**Vectors**
- [X] Dot product — formula, geometric meaning, when result is 0 / positive / negative
- [ ] Cross product — formula, right-hand rule, geometric meaning
- [X] Normalize — why and when you need unit vectors
- [X] Projection of one vector onto another

**Matrices**
- [ ] Why matrix multiplication is not commutative
- [X] How to read a transformation matrix (what each row/column means)
- [X] Why we use 4×4 instead of 3×3
- [X] What the `w` component means for points vs vectors
- [ ] How to invert a rotation matrix (transpose — and why this works)

**Coordinate Spaces**
- [X] What each space in the pipeline means and why it exists
- [X] How the view matrix is constructed from camera position and orientation
- [ ] Why `view = inverse(camera_to_world)`
- [X] What the frustum is and how the projection matrix warps it into a unit cube
- [ ] What the perspective divide does and why

**The Pipeline**
- [ ] Write the MVP chain from memory: `clip = P * V * M * local`
- [ ] Explain what each matrix contributes
- [ ] Go from a 3D point to a screen pixel coordinate by hand

---

## Checkpoint

**You are done with Phase 1 when:**

A 3D wireframe cube, constructed from 8 hand-defined vertices and 12 edges, is correctly projected onto a 2D canvas from a camera you control (position and look-at target), with perspective (far edges smaller than near edges). Output as either a PPM file or an SDL2 window.

If it looks right and you can explain why every step works, move to Phase 2.

---

## Notes on Pitfalls

**Row-major vs column-major confusion** is the #1 source of bugs in Phase 1. Decide your convention on Day 1 and document it in a comment at the top of your `mat4.h`. Don't switch mid-project.

**Gimbal lock** will bite you if you implement rotation with three separate Euler angle rotations applied in sequence. For Phase 1 this is fine. Just be aware it exists.

**The projection matrix derivation** looks intimidating but it's really just similar triangles + some algebraic rearranging to fit matrix form. Scratchapixel walks through it step by step. Do the derivation yourself on paper before coding it.

**Near/far clipping planes** — don't set near = 0. The projection matrix divides by `near` internally; a zero near plane causes division by zero and breaks depth precision. A near value of `0.1` is a safe default.