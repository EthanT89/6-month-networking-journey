#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"
#include "./texture.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

static float clamp_uv(float val) {
    if (val < 0.0f) return 0.0f;
    if (val > 1.0f) return 1.0f;
    return val;
}

void texture_sample_nn(float u, float v, struct Texture texture, unsigned char *r, unsigned char *g, unsigned char *b){
    u = clamp_uv(u);
    v = clamp_uv(v);

    int x = u * (texture.width - 1);
    int y = v * (texture.height - 1);

    int idx = 3 * (( y * texture.width ) + x );
    
    *r = texture.pixel_data[idx];
    *g = texture.pixel_data[idx+1];
    *b = texture.pixel_data[idx+2];
}

void texture_load(const char *filename, struct Texture *texture){
    int width, height, channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if (data == NULL) {
        printf("[TEXTURE] Failed to load: %s\n", filename);
        return;
    }

    texture->height = height;
    texture->width = width;
    texture->pixel_data = data;
}

void texture_sample_bl(float u, float v, struct Texture texture, unsigned char *r, unsigned char *g, unsigned char *b){
    // unimplemented, TODO
    int idx = u + v;

    *r = texture.pixel_data[idx];
    *g = texture.pixel_data[idx+1];
    *b = texture.pixel_data[idx+2];
}

void texture_destroy(struct Texture *texture){
    free(texture->pixel_data);
}