# Phase 1 — Step 3: Coordinate Spaces & The Camera

> This is the conceptual core of the entire renderer. Everything before this — vectors, matrices, transforms — was building the vocabulary. This is where you use it to answer the central question of 3D graphics: **how does a 3D world become a 2D image?**

---

## The Big Picture First

Before diving into any math, understand the *journey* a vertex takes from the moment you define it to the moment it appears as a pixel on screen. Every stage exists for a reason.

```
Vertex in model file
        ↓  model matrix
World space
        ↓  view matrix (lookAt)
Camera space
        ↓  projection matrix
Clip space
        ↓  perspective divide (÷w)
NDC space
        ↓  viewport transform
Screen space (pixels)
```

Each arrow is a matrix multiply (except the perspective divide, which is explicit). Each space answers a different question:

- *Where is this vertex relative to its own object?* → model space
- *Where is it in the world?* → world space
- *Where is it relative to the camera?* → camera space
- *Is it visible? Where on the image plane does it project?* → clip / NDC space
- *Which pixel does it land on?* → screen space

You've already built the tools for every transform. Now you need to understand what each one is actually doing.

---

## Part 1 — Model Space and World Space

### Model Space (Object Space)

When an artist models a cube, they define it centered at the origin. The 8 vertices of a unit cube are:

```
(-1, -1, -1)   (1, -1, -1)
(-1,  1, -1)   (1,  1, -1)
(-1, -1,  1)   (1, -1,  1)
(-1,  1,  1)   (1,  1,  1)
```

This is model space. The object doesn't "know" where it is in the world — it just knows its own shape, defined relative to its own local origin.

### The Model Matrix — Placing Objects in the World

The model matrix is composed of the transforms you've already built:

```c
struct Matrix model = mat4_mul(translation, mat4_mul(rotation, scale));
```

Order matters here — and it's a common source of confusion.

Matrix multiplication is not commutative: `A * B ≠ B * A`. For transforms, the convention is:

```
model = Translation * Rotation * Scale
```

Read right to left — scale first, then rotate, then translate. Why?

Think about it physically. If you scale first, the object grows around its own origin. If you then rotate, it spins around that same origin. If you then translate, the whole thing moves to its final position in the world. Do it in the wrong order and you get different results — translating first and then scaling would move the object and then scale it relative to the world origin, dragging it further from where you put it.

**Key insight:** The model matrix encodes the answer to "where is this object in the world, how is it oriented, and how big is it?" Multiplying a model-space vertex by the model matrix gives you its position in world space.

---

## Part 2 — Camera Space and the View Matrix

### What Camera Space Is

Camera space (also called view space or eye space) is the world as seen from the camera's perspective. By convention:

- The camera sits at the **origin** `(0, 0, 0)`
- The camera points down the **negative Z axis**
- **Y is up**, **X is right**

Why negative Z? Convention. It means objects in front of the camera have negative Z coordinates in camera space, which simplifies the math for clipping and projection.

### The Core Insight About the View Matrix

Here's the thing most explanations get wrong or gloss over:

**The view matrix does not move the camera. It moves the entire world.**

Think about it this way. Imagine you're holding a camera and you walk to the right. From your perspective, the scene moves to the left. Both descriptions are equivalent — you moved right, or the world moved left. The view matrix takes the second interpretation: it transforms every vertex in the scene so that the camera ends up at the origin.

This is why:
```
view = inverse(camera_to_world_transform)
```

The camera has a position and orientation in the world — that's its "camera-to-world" transform. The view matrix is the *inverse* of that transform. It undoes the camera's position and orientation, which is equivalent to moving the world so the camera ends up at the origin.

### Constructing the View Matrix — lookAt

`lookAt` takes three inputs:

```c
struct Matrix lookAt(struct Vector3 eye, struct Vector3 target, struct Vector3 up);
```

- `eye` — where the camera is in world space
- `target` — the point the camera is looking at
- `up` — which direction is "up" for the camera (usually `(0, 1, 0)`)

From these three vectors, you need to construct a **coordinate frame** — three perpendicular axes that define the camera's orientation. This is the geometric heart of `lookAt`.

#### Step 1 — The Forward Axis (Z)

The camera looks toward the target. The forward direction is:

```
forward = normalize(eye - target)
```

Note: `eye - target`, not `target - eye`. This gives you a vector pointing *away* from the target — in the direction the camera's back is facing. This aligns with the convention that the camera looks down **negative Z**, so the positive Z axis of the camera points behind it.

#### Step 2 — The Right Axis (X)

You have the forward direction and you know which way is up (the `up` hint). The right axis is perpendicular to both:

```
right = normalize(cross(up, forward))
```

Wait — why `cross(up, forward)` and not `cross(forward, up)`? Cross product is not commutative — `cross(A, B) = -cross(B, A)`. The order determines the direction of the result by the right-hand rule. You want `right` to point to the camera's right, so the order matters. Think through it with the right-hand rule — point your fingers in the `up` direction, curl them toward `forward`, and your thumb points right.

**Important:** The `up` input is a *hint*, not the final up vector. It just tells the function which general direction is up. The actual up vector for the camera's frame is computed in the next step.

#### Step 3 — The True Up Axis (Y)

Now that you have `forward` and `right`, the true up axis is perpendicular to both:

```
true_up = cross(forward, right)
```

You don't need to normalize this one — since `forward` and `right` are already unit vectors and perpendicular to each other, their cross product is automatically unit length.

#### Step 4 — Building the Matrix

You now have three orthogonal unit vectors: `right`, `true_up`, `forward`. These become the rows of your view matrix. Each row "projects" a world-space position onto one of the camera's axes.

```
| right.x     right.y     right.z     -dot(right,   eye) |
| true_up.x   true_up.y   true_up.z   -dot(true_up, eye) |
| forward.x   forward.y   forward.z   -dot(forward, eye) |
| 0           0           0            1                  |
```

The translation column (`-dot(axis, eye)`) handles moving the world so the camera ends up at the origin. It's the dot product of each camera axis with the eye position, negated — mathematically equivalent to applying the inverse translation after the rotation.

#### Why This Works

Each row of a matrix, when dotted with an input vector, asks: "how much of this input vector lies along this axis?" By putting `right` in row 0, you're asking "how much of this world-space point lies along the camera's right axis?" — which gives you the point's X coordinate in camera space. Same for Y and Z. The translation terms shift the origin from world-space origin to the camera's eye position.

#### Verify Your lookAt

Test case: camera at `(0, 0, 5)`, looking at `(0, 0, 0)`, up is `(0, 1, 0)`.

- `forward = normalize((0,0,5) - (0,0,0)) = (0, 0, 1)`
- `right = normalize(cross((0,1,0), (0,0,1))) = (1, 0, 0)`
- `true_up = cross((0,0,1), (1,0,0)) = (0, 1, 0)`

Translation terms:
- `-dot(right, eye) = -dot((1,0,0), (0,0,5)) = 0`
- `-dot(true_up, eye) = -dot((0,1,0), (0,0,5)) = 0`
- `-dot(forward, eye) = -dot((0,0,1), (0,0,5)) = -5`

Resulting view matrix:
```
| 1  0  0   0 |
| 0  1  0   0 |
| 0  0  1  -5 |
| 0  0  0   1 |
```

Now multiply a point at the origin `(0, 0, 0, 1)` through this matrix. You should get `(0, 0, -5, 1)` — the origin is now 5 units in front of the camera (at -5 in Z, because the camera looks down -Z). That's exactly right.

---

## Part 3 — The Projection Matrix

### What Projection Does

You've taken a 3D point all the way from model space to camera space. Now you need to flatten it onto a 2D image plane. This is projection.

The core geometric idea is simple: rays extend from every point in the scene toward the camera's eye. Where those rays intersect the image plane is where the point "appears" in the image. Objects further away produce rays that intersect the image plane closer to the center — which is why far things appear smaller. This is perspective foreshortening, and it's the entire basis of how we perceive depth.

### The Viewing Frustum

Before diving into the matrix, understand what the projection matrix is actually transforming *from* and *to*.

The **viewing frustum** is the region of space that's visible to the camera. It's a truncated pyramid defined by:

- **Near plane** — the closest distance anything can be to the camera to be visible (e.g. `0.1`)
- **Far plane** — the furthest distance (e.g. `1000.0`)
- **Field of view (FOV)** — the angle of the camera's view cone (e.g. `90°`)
- **Aspect ratio** — width divided by height of the screen

Everything inside this frustum is potentially visible. Everything outside gets clipped.

The projection matrix transforms this frustum into a **unit cube** — a box spanning `[-1, 1]` in X and Y, and `[0, 1]` or `[-1, 1]` in Z depending on convention. This standardized box is called **clip space**, and then after the perspective divide it becomes **NDC space**.

Why transform to a unit cube? Because clipping (discarding geometry outside the view) is trivially easy in this space — just check if coordinates are in `[-1, 1]`. No frustum-specific math needed.

### Deriving the Projection Matrix

Don't memorize this. Understand it.

#### The Perspective Divide — the Core Idea

In camera space, a point is at `(x, y, z)`. The camera is at the origin looking down -Z. The image plane is at `z = -near`.

By similar triangles (the same geometry Dürer used in the 15th century to understand perspective drawing):

```
x_projected = x * near / (-z)
y_projected = y * near / (-z)
```

The division by `-z` is the perspective divide. It's what makes far things small — a point twice as far away has twice the `-z`, so its projected x and y are halved.

This single division is the entire mathematical basis of perspective. Everything else is bookkeeping to make it fit inside a matrix.

#### Making It a Matrix — The w Trick

You can't do a division inside a matrix multiply. But you can *store* the value to divide by in `w`, and do the division afterward. This is the perspective divide step.

The projection matrix is designed so that after multiplying a camera-space point, the resulting `w` contains the original `-z` value. Then you divide x, y, z by w to get NDC coordinates.

```c
// After projection matrix multiply:
vec4 clip = mat4_mul_vec4(projection, camera_space_point);

// Perspective divide:
float x_ndc = clip.x / clip.w;
float y_ndc = clip.y / clip.w;
float z_ndc = clip.z / clip.w;
```

#### Remapping to NDC

The projection matrix also remaps the frustum to the unit cube. The x and y coordinates need to be remapped from `[-right, right]` (the frustum's extent at the near plane) to `[-1, 1]`. The z coordinate needs to be remapped from `[-near, -far]` to `[-1, 1]` (or `[0, 1]`).

For a symmetric frustum (the common case), the half-width at the near plane is:

```
half_height = near * tan(fov / 2)
half_width  = half_height * aspect_ratio
```

The x remapping gives you `m[0][0]`:
```
m[0][0] = near / half_width = 1 / (aspect * tan(fov/2))
```

The y remapping gives you `m[1][1]`:
```
m[1][1] = near / half_height = 1 / tan(fov/2)
```

The z remapping (to `[-1, 1]`) gives you `m[2][2]` and `m[2][3]`:
```
m[2][2] = -(far + near) / (far - near)
m[2][3] = -(2 * far * near) / (far - near)
```

And the w trick — storing `-z` into `w`:
```
m[3][2] = -1
```

The full perspective projection matrix:

```
| 1/(aspect*tan(fov/2))    0              0                          0                      |
| 0                         1/tan(fov/2)  0                          0                      |
| 0                         0             -(far+near)/(far-near)     -(2*far*near)/(far-near)|
| 0                         0             -1                         0                      |
```

#### Verify It

Take a point exactly at the near plane: `(0, 0, -near, 1)`. After multiplying by this matrix and doing the perspective divide, it should land at `(0, 0, -1)` in NDC — the near face of the unit cube.

Take a point exactly at the far plane: `(0, 0, -far, 1)`. After the perspective divide, it should land at `(0, 0, 1)` — the far face.

Work through the arithmetic yourself. If you get those results, your matrix is correct.

---

## Part 4 — NDC to Screen Space

After the perspective divide you're in NDC space: x and y are in `[-1, 1]`. You need to map this to pixel coordinates.

```
pixel_x = (x_ndc + 1) / 2 * screen_width
pixel_y = (1 - y_ndc) / 2 * screen_height
```

Note the Y flip — NDC has Y increasing upward, but screen coordinates typically have Y increasing downward (top-left is origin). The `(1 - y_ndc)` handles this.

---

## Part 5 — The Full Pipeline in Code

Putting it all together:

```c
// 1. Build matrices
struct Matrix model = mat4_mul(
    translation_constructor(5.0f, 0.0f, 0.0f),
    mat4_mul(
        rotation_y_constructor(0.785f), // 45 degrees
        scale_constructor(1.0f, 1.0f, 1.0f)
    )
);

struct Vector3 eye    = {0, 0, 5};
struct Vector3 target = {0, 0, 0};
struct Vector3 up     = {0, 1, 0};
struct Matrix view = lookAt(eye, target, up);

struct Matrix projection = perspective(
    1.5708f,  // 90 degrees FOV in radians
    16.0f/9.0f, // aspect ratio
    0.1f,     // near
    1000.0f   // far
);

// 2. Compose MVP
struct Matrix vp  = mat4_mul(projection, view);
struct Matrix mvp = mat4_mul(vp, model);

// 3. Transform a vertex
struct Vector4 local_pos = {1.0f, 1.0f, -1.0f, 1.0f}; // w=1, it's a point
struct Vector4 clip_pos  = mat4_mul_vec4(mvp, local_pos);

// 4. Perspective divide
float x_ndc = clip_pos.x / clip_pos.w;
float y_ndc = clip_pos.y / clip_pos.w;
float z_ndc = clip_pos.z / clip_pos.w;

// 5. Viewport transform
int screen_width  = 800;
int screen_height = 600;
int pixel_x = (int)((x_ndc + 1.0f) / 2.0f * screen_width);
int pixel_y = (int)((1.0f - y_ndc) / 2.0f * screen_height);
```

That's the entire pipeline. Every vertex in your scene goes through exactly these steps.

---

## Part 6 — Concepts to Sit With

### Why is matrix order reversed from what you'd expect?

```
clip = projection * view * model * local
```

Reading left to right, you apply projection last. But logically, model comes first. This is because vectors are column vectors on the right — the rightmost matrix is applied first. Some engines/APIs use row vectors and reverse the order. This is one of the most common sources of bugs when switching between APIs.

### What does the view matrix really look like geometrically?

The view matrix is a rigid body transform — pure rotation and translation, no scale. Its upper-left 3×3 contains three orthonormal vectors (the camera axes). This means it's an orthogonal matrix, which has a beautiful property: its inverse is its transpose. So:

```
inverse(view_rotation) = transpose(view_rotation)
```

You don't need a general matrix inverse for the view matrix — just transpose the 3×3 rotation part and negate the translation.

### Why does the projection matrix have -1 in row 3, column 2?

This is the `w = -z` trick. After multiplying, `clip.w = -camera_z`. Since points in front of the camera have negative Z in camera space, `-z` is positive — which is what you want for the perspective divide. It ensures the divide produces correct results for visible geometry.

### What is "clipping" and why does it happen in clip space?

After the projection matrix multiply (but before the perspective divide), you're in clip space. At this stage, a point is visible if:

```
-w <= x <= w
-w <= y <= w
-w <= z <= w  (or 0 <= z <= w depending on convention)
```

This is easy to check without knowing anything about the frustum's specific geometry. Geometry that straddles these boundaries gets clipped — new triangles are generated at the intersection. This happens before the perspective divide because dividing by w for points behind the camera (w ≤ 0) would produce inverted results.

---

## Part 7 — Common Pitfalls

**FOV in degrees vs radians.** `tan()` in C takes radians. If you pass degrees, you get garbage. Always convert: `fov_radians = fov_degrees * M_PI / 180.0f`.

**Near plane = 0.** Division by zero in the projection matrix. Use `0.1f` minimum.

**Not normalizing the lookAt vectors.** If `forward`, `right`, or `true_up` aren't unit length, your view matrix will scale the scene unexpectedly. Always normalize.

**Wrong matrix multiply order.** `mvp = projection * view * model`, not the reverse. Test with a known point and verify the result makes geometric sense.

**Y axis inversion in screen space.** NDC Y goes up, screen Y goes down. Forgetting the flip puts everything upside down.

**Aspect ratio inverted.** `width / height`, not `height / width`. An inverted aspect ratio squishes the image in the wrong direction.

---

## Checkpoint

Before writing `lookAt` and `perspective`, work through these by hand:

1. Camera at `(0, 0, 5)`, looking at origin. What is the view matrix? Multiply `(0, 0, 0, 1)` through it — where does the origin end up in camera space?

2. A point at `(0, 0, -10)` in camera space, FOV 90°, aspect 1.0, near 0.1, far 100. What are its NDC coordinates after projection and perspective divide?

3. That NDC point on an 800×600 screen — what pixel does it land on?

If you can work through all three from memory, you're ready to implement.