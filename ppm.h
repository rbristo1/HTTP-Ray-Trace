#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "vec3.h"

/**
 * Write to a raytraced output (RTO) format.
 * @param pixels an array of pixels in row-major order. This must have at least width * height pixels.
 * @param width  the number of columns in the pixels.
 * @param height the number of rows in the pixels.
 * @param fout   the output file to write the RTO format.
 * @return the number of pixels written to the output file. 
 */
size_t ppm_write(const Pixel *pixels, uint16_t width, uint16_t height, FILE *fout);

