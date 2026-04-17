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
    int width; // pixel width
    int height; // pixel height
    unsigned char *pixel_data; // total size = width * height * 3
};

/*
 * given the u and v values for a pixel, determine the corresponding texture map rgb value (rounds to nearest texture pixel)
 */
void texture_sample_nn(float u, float v, struct Texture texture, unsigned char *r, unsigned char *g, unsigned char *b);

/*
 * load a texture file into the texture struct
 */
void texture_load(const char *filename, struct Texture *texture);

/*
 * given the u and v values for a pixel, determine the corresponding texture map rgb value (averages between 4 nearest neighbors)
 */
void texture_sample_bl(float u, float v, struct Texture *texture, unsigned char *r, unsigned char *g, unsigned char *b);

/*
 * free a texture structure
 */
void texture_destroy(struct Texture *texture);

#endif