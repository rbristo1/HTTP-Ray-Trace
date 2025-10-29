#pragma once
#include "vec3.h"
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

/**
 * Lookup and write the pixels to a plugin library.
 * @param output_file the file name to write the pixels to.
 * @param output_type the type of file from Options, either bmp, ppm, or rto.
 * @param width the number of columns in the image.
 * @param height the number of rows in the image.
 * @param pixels the pixels to write to the file
 * @return -1 on error, number of pixels written otherwise.
 */
ssize_t plugin_write(const char *output_file,
                     const char *output_type,
                     uint16_t width,
                     uint16_t height,
                     const Pixel *pixels);

ssize_t plugin_write_stream(FILE *output_stream,
                            const char *output_type,
                            uint16_t width,
                            uint16_t height,
                            const Pixel *pixels);