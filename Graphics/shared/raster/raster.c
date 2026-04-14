#include "./raster.h"

void draw_line(struct Framebuffer *fb, 
               int x0, int y0, int x1, int y1,
               unsigned char r, unsigned char g, unsigned char b)
{
    int tempx;
    int tempy;

    int negative_slope = 0;
    int steep = 0;


    if (x1 - x0 < 0){ // line goes backwards, reverse it to make algo simpler
        tempx = x0;
        tempy = y0;
        x0 = x1;
        y0 = y1;
        x1 = tempx;
        y1 = tempy;
    }

    if (y1 - y0 < 0){ // slope is negative, reflect over X axis and later correct this when applying pixel value
        negative_slope = 1;
        y0 *= -1;
        y1 *= -1;
    }

    if ( y1-y0 > x1-x0){ // slope is greater than 1, swap X and Y, swap back before applying pixel value
        steep = 1;
        tempx = x0; // swap first pair
        x0 = y0;
        y0 = tempx;

        tempx = x1; // swap last pair
        x1 = y1;
        y1 = tempx;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    int X = x0;
    int Y = y0;
    // note: algo assumes all lines proceed left to right, the earlier check guarantees this.
    int d = 2*dy - dx;
    int dE = 2*dy;
    int dNE = 2*(dy-dx);
    while(X <= x1){
        int realX = X;
        int realY = Y;

        if (steep){ // earlier we swapped x and y since the slope was so steep, we revert back before applying pixel colors
            realX = Y;
            realY = X;
        }

        if (negative_slope){
            realY *= -1;
        }

        fb_set_pixel(fb, realX, realY, r, g, b);
        X++;

        if (d <= 0){
            d += dE;
        } else {
            Y++;
            d += dNE;
        }
    }
}

void draw_triangle(struct Framebuffer *fb, 
                   struct Vector4 v0, struct Vector4 v1, struct Vector4 v2, 
                   float b0, float b1, float b2,
                   unsigned char r, unsigned char g, unsigned char b)
{

    // 1. **Bounding box** — find min/max x and y across all three vertices. Clamp to framebuffer bounds.
    int minx = MIN(v0.x, MIN(v1.x, v2.x));
    int miny = MIN(v0.y, MIN(v1.y, v2.y));
    int maxx = MAX(v0.x, MAX(v1.x, v2.x));
    int maxy = MAX(v0.y, MAX(v1.y, v2.y));

    minx = MAX(minx, 0);
    miny = MAX(miny, 0);
    maxx = MIN(maxx, fb->width - 1);
    maxy = MIN(maxy, fb->height - 1);

    // 2. **Barycentric coordinates** — for each pixel in the bounding box, compute the barycentric weights. Use the signed area method.
    float total_area = signed_area(v0, v1, v2);
    if (fabsf(total_area) < 1e-6f) return;

    for (int x = minx; x <= maxx; x++){
        for (int y = miny; y <= maxy; y++){

            struct Vector3 p = {x,y,0}; // initialize to 0 depth because we don't have that yet.

            // unnormalized weights (before dividing by total_area)
            float w0 = signed_area(p, v1, v2);
            float w1 = signed_area(v0, p, v2);
            float w2 = signed_area(v0, v1, p);

            // 3. **Inside test** — if all three weights are ≥ 0, the pixel is inside the triangle.
            if (w0 > 0 || w1 > 0 || w2 > 0) continue; // inside check, discard if outside

            // normalized weights (after dividing)
            float alpha = w0 / total_area;
            float beta = w1 / total_area;
            float gamma = w2 / total_area;
            
            // interpolate the brightness of the pixel based each vertex normal's brightness
            float interpolated_b = alpha * b0 + beta * b1 + gamma * b2;

            // 4. **Depth interpolation** — interpolate the depth value using barycentric weights. Use perspective-correct interpolation.
            p.z = alpha * v0.z + beta * v1.z + gamma * v2.z; // naive approach, currently not using w_clipspace
            
            // 5. **Depth test** — compare interpolated depth against the depth buffer. Only proceed if closer.
            if ( fb_depth_test(fb, x, y, p.z) == 0) continue;

            // 6. **Write pixel** — update color buffer and depth buffer.
            fb_set_pixel(fb, x, y, r * interpolated_b, g * interpolated_b, b * interpolated_b);
        }
    }
}