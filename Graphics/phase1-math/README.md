# Phase 1 — Math Foundation

**Goal:** Build a complete linear algebra library in C from scratch.

## Objectives

- [ ] `vec3`, `vec4` — dot, cross, normalize, magnitude
- [ ] `mat4` — multiply, transpose, identity
- [ ] Translation, rotation, scale matrices
- [ ] View matrix (lookAt construction)
- [ ] Perspective & orthographic projection matrices
- [ ] NDC → screen space mapping

## Checkpoint

A 3D point projected correctly onto a 2D canvas. All 8 vertices of a cube visible as dots.

## Key insight to internalize

Every transform is a matrix. Chaining transforms is matrix multiplication. The order matters — and understanding *why* it matters is the unlock.

## Log

See `log/` for daily notes.
