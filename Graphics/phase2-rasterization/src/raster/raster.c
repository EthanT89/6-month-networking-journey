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