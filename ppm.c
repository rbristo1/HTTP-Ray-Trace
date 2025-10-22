#include "ppm.h"

size_t ppm_write(const Pixel *pixels, uint16_t width, uint16_t height, FILE *fout)
{
    size_t ret = 0;
    fprintf(fout, "P3\n");
    fprintf(fout, "%d %d\n", width, height);
    fprintf(fout, "%d\n", 255);
    int numPixels = (int)width * (int)height;
    for (int i = 0; i < numPixels; i++) {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        r = pixels[i].r*256;
        g = pixels[i].g*256;
        b = pixels[i].b*256;
        if (((i+1)%(width)) != 0 || i == 0) {
            fprintf(fout, "%-3d %-3d %-3d    ", r, g, b);
        }
        else {
            fprintf(fout, "%-3d %-3d %-3d\n", r, g, b);
        }
        

        ret++;
    }
    return ret;
}

