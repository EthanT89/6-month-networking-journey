/*
struct Texture        — width, height, pointer to pixel data
texture_load()        — loads an image file using stb_image
texture_sample_nn()   — nearest-neighbor sampling at (u,v)
texture_sample_bl()   — bilinear sampling at (u,v) [stretch goal]
texture_destroy()     — frees pixel data
*/

#ifndef TEXTURE_H
#define TEXTURE_H

struct Texture {
    int width;
    int height;
    unsigned char *pixel_data;
};

void texture_sample_nn(float u, float v, struct Texture texture,unsigned char *r, unsigned char *g, unsigned char *b);

void texture_load(unsigned char *filename, struct Texture *texture);

void texture_sample_bl(float u, float v, struct Texture texture,unsigned char *r, unsigned char *g, unsigned char *b);

void texture_destroy(struct Texture *texture);

#endif