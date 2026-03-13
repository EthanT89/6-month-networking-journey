# Graphics — Deep Work Journey (Month 4–5)

A software renderer built from scratch in C. No OpenGL. No GPU shortcuts. Every stage of the 3D → 2D pipeline implemented by hand.

## Why this exists

After 3 months of network programming, I hit a wall: I needed something to *integrate* that knowledge into. Graphics is that thing. It touches systems programming, linear algebra, physics, and eventually — networking (multiplayer rendering, networked simulations).

## Project structure

```
Graphics/
├── shared/          # Math lib and assets reused across all phases
├── phase1-math/     # Vectors, matrices, coordinate spaces
├── phase2-raster/   # Triangle rasterization, z-buffer, interpolation
├── phase3-lighting/ # Normals, Phong/Blinn-Phong, multiple lights
├── phase4-textures/ # Texture mapping, shadow mapping, final scene
└── docs/            # Planning docs, references, research notes
```

## The pipeline

```
3D World → Vertex Transform → Clipping → Rasterization → Fragment Shading → Framebuffer
```

Each phase is a self-contained C project. Shared math lives in `shared/math/` and is symlinked or copied into each phase as needed.

## Status

| Phase | Focus | Status |
|---|---|---|
| Phase 1 | Math foundation (vec, mat, transforms) | Not started |
| Phase 2 | Rasterization & z-buffer | Not started |
| Phase 3 | Lighting (Phong, Blinn-Phong) | Not started |
| Phase 4 | Textures & shadow mapping | Not started |

## Connection to the Network journey

Months 1–3 built TCP/UDP fundamentals and a distributed task queue in C. This phase expands the toolkit — the endgame is a networked simulation or multiplayer environment that draws on both.
