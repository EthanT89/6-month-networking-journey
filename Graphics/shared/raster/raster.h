#ifndef RASTER_H
#define RASTER_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#include "../framebuffer/framebuffer.h"
#include "../../phase4-textures-shadows/src/texture/texture.h"
#include "../math/vec/vector.h"
#include "../geometry/geometry.h"
#include <math.h>

void draw_line(struct Framebuffer *fb, 
               int x0, int y0, int x1, int y1,
               unsigned char r, unsigned char g, unsigned char b);

void draw_triangle(struct Framebuffer *fb, 
                   struct Vector3 v0, struct Vector3 v1, struct Vector3 v2,
                   struct Vector2 uv0, struct Vector2 uv1, struct Vector2 uv2,  
                   float b0, float b1, float b2,
                   float w0, float w1, float w2,
                   struct Texture tex);

#endif