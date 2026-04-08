#ifndef RASTER_H
#define RASTER_H


#include "../framebuffer/framebuffer.h"

void draw_line(struct Framebuffer *fb, 
               int x0, int y0, int x1, int y1,
               unsigned char r, unsigned char g, unsigned char b);

#endif