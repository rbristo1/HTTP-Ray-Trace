#include "plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
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
                     const Pixel *pixels) 
{
    FILE * out = fopen(output_file, "w");
    char pluginName[256];
    strcpy(pluginName, "./lib/lib");
    strcat(pluginName, output_type);
    strcat(pluginName, ".so");
    
    ssize_t (*func)(const Pixel*, uint16_t, uint16_t, FILE *);
    void * descriptor = dlopen(pluginName, RTLD_LAZY | RTLD_LOCAL);
    if (!descriptor) {
        return -1;
    }
    char output_type_full[256];
    strcpy(output_type_full, output_type);
    strcat(output_type_full, "_write");
    dlerror();
    func = dlsym(descriptor, output_type_full);
    char* error = dlerror();
    if (error != NULL) {
        dlclose(descriptor);
        return 2;
    }
    ssize_t ret = func(pixels, width, height, out);
    fclose(out);

    dlclose(descriptor);

    return ret;


        
}