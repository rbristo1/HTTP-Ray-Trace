#include "bmp.h"
#include <stdlib.h>

#pragma pack(push,2)
struct BFH {
    char type[2];
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};
#pragma pack(pop)
#pragma pack(push,2)
struct BIH {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitcount;
    uint32_t compression;
    uint32_t imagesize;
    uint32_t x_ppm;
    uint32_t y_ppm;
    uint32_t color_used;
    uint32_t color_important;
};
#pragma pack(pop)

size_t bmp_write(const Pixel *pixels, uint16_t width, uint16_t height, FILE *fout)
{
    size_t ret = 0;
    //size_t pixels_written = 0;
    uint8_t padding = 0;
    struct BFH * bfh = malloc(sizeof(struct BFH));
    bfh->type[0] = 'B';
    bfh->type[1] = 'M';
    bfh->size = ((width*height)*3)+54;
    bfh->reserved1 = 0;
    bfh->reserved2 = 0;
    bfh->offset = 54;
    struct BIH * bih = malloc(sizeof(struct BIH));
    bih->size = 40;
    bih->width = width;
    bih->height = height;
    bih->planes = 1;
    bih->bitcount = 24;
    bih->compression = 0;
    bih->imagesize = ((width*height)*3);
    bih->x_ppm = 0;
    bih->y_ppm = 0;
    bih->color_used = 0;
    bih->color_important = 0;
    fwrite(bfh, sizeof(struct BFH), 1, fout);
    fwrite(bih, sizeof(struct BIH), 1, fout);
    //printf("<%d> <%d>", width, height);
    int paddingAmount = (4-((width*3)%4))%4;
    //printf("%d", paddingAmount);
    //printf("Testing raytracer output (.rto) writer.");
    for (int i = height-1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            uint8_t b = pixels[i*width+j].b*255;
            uint8_t g = pixels[i*width+j].g*255;
            uint8_t r = pixels[i*width+j].r*255;

            fwrite(&b, sizeof(uint8_t), 1, fout);
            fwrite(&g, sizeof(uint8_t), 1, fout);
            fwrite(&r, sizeof(uint8_t), 1, fout);
            ret++;
        }
        if (paddingAmount != 0) {
            for (int j = 0; j < paddingAmount; j++) {
                fwrite(&padding, sizeof(uint8_t), 1, fout);
            }
        }
        
        
    }
    free(bih);
    free(bfh);
    return ret;
}
