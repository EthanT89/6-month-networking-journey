#ifndef RASTER_H
#define RASTER_H

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#include "../framebuffer/framebuffer.h"
#include "../../phase4-textures-shadows/src/texture/texture.h"
#include "../../phase4-textures-shadows/src/shadow/shadow.h"
#include "../math/vec/vector.h"
#include "../geometry/geometry.h"
#include <math.h>

void draw_line(struct Framebuffer *fb, 
               int x0, int y0, int x1, int y1,
               unsigned char r, unsigned char g, unsigned char b);

void draw_triangle_textured(struct Framebuffer *fb, 
                            struct Triangle triangle,
                            struct ShadowMap *shadow_map,
                            struct Texture tex);

void draw_triangle(struct Framebuffer *fb, 
                   struct Triangle triangle);

#endif