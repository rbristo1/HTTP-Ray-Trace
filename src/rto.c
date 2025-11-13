#include "rto.h"
/**
 * Write to a raytraced output (RTO) format.
 * @param pixels an array of pixels in row-major order. This must have at least width * height pixels.
 * @param width  the number of columns in the pixels.
 * @param height the number of rows in the pixels.
 * @param fout   the output file to write the RTO format.
 * @return the number of pixels written to the output file. 
 */
size_t rto_write(const Pixel *pixels, uint16_t width, uint16_t height, FILE *fout)
{
    size_t pixels_written = 0;
    
    fwrite(&width, sizeof(uint16_t), 1, fout);
    
    fwrite(&height, sizeof(uint16_t), 1, fout);
    size_t pixelCount = (size_t)width*(size_t)height;
    printf("<%d> <%d>", width, height);
    //printf("Testing raytracer output (.rto) writer.");
    for (size_t i = 0; i < pixelCount; i++) {
        uint8_t r = pixels[i].r*255;
        uint8_t g = pixels[i].g*255;
        uint8_t b = pixels[i].b*255;
        int x = i%width;
        int y = i/width;
        //printf("<%d,%d %d> <%d,%d %d> <%d,%d %d>\n", x, y, r, x, y, g, x, y, b);
        fwrite(&r, sizeof(uint8_t), 1, fout);
        fwrite(&g, sizeof(uint8_t), 1, fout);
        fwrite(&b, sizeof(uint8_t), 1, fout);
        pixels_written++;
    }
    //fclose(fout);
    return pixels_written;
}
