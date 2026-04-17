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

void texture_sample_bl(float u, float v, struct Texture *texture, unsigned char *r, unsigned char *g, unsigned char *b){
    u = clamp_uv(u);
    v = clamp_uv(v);

    float px = u * (texture->width  - 1);
    float py = v * (texture->height - 1);

    int x0 = (int)px;
    int y0 = (int)py;
    int x1 = x0 + 1 < texture->width  ? x0 + 1 : x0;
    int y1 = y0 + 1 < texture->height ? y0 + 1 : y0;

    float fx = px - x0;
    float fy = py - y0;

    // sample the four surrounding texels
    int i00 = (y0 * texture->width + x0) * 3;
    int i10 = (y0 * texture->width + x1) * 3;
    int i01 = (y1 * texture->width + x0) * 3;
    int i11 = (y1 * texture->width + x1) * 3;

    // blend horizontally across top row
    float top_r = texture->pixel_data[i00+0] + fx * (texture->pixel_data[i10+0] - texture->pixel_data[i00+0]);
    float top_g = texture->pixel_data[i00+1] + fx * (texture->pixel_data[i10+1] - texture->pixel_data[i00+1]);
    float top_b = texture->pixel_data[i00+2] + fx * (texture->pixel_data[i10+2] - texture->pixel_data[i00+2]);

    // blend horizontally across bottom row
    float bot_r = texture->pixel_data[i01+0] + fx * (texture->pixel_data[i11+0] - texture->pixel_data[i01+0]);
    float bot_g = texture->pixel_data[i01+1] + fx * (texture->pixel_data[i11+1] - texture->pixel_data[i01+1]);
    float bot_b = texture->pixel_data[i01+2] + fx * (texture->pixel_data[i11+2] - texture->pixel_data[i01+2]);

    // blend vertically between top and bottom
    *r = (unsigned char)(top_r + fy * (bot_r - top_r));
    *g = (unsigned char)(top_g + fy * (bot_g - top_g));
    *b = (unsigned char)(top_b + fy * (bot_b - top_b));
}

void texture_destroy(struct Texture *texture){
    free(texture->pixel_data);
}