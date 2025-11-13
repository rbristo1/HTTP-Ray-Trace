#include "plugin.h"
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
ssize_t plugin_write_stream(FILE *output_stream,
                            const char *output_type,
                            uint16_t width,
                            uint16_t height,
                            const Pixel *pixels) {

    char pluginName[256];

    //creates string of path to the DSO needed
    strcpy(pluginName, "./lib/lib");
    strcat(pluginName, output_type);
    strcat(pluginName, ".so");

    //dynamic function definition, 
    //returns a ssize_t, 
    //takes parameters const Pixel*, uint16_t, uint16_t, and FILE *
    ssize_t (*func)(const Pixel*, uint16_t, uint16_t, FILE *);

    //opens DSO
    void * descriptor = dlopen(pluginName, RTLD_LAZY | RTLD_LOCAL);
    if (!descriptor) {
        return -1;
    }

    //builds string for the function name to be run
    char output_type_full[256];
    strcpy(output_type_full, output_type);
    strcat(output_type_full, "_write");

    //clears errors if any
    dlerror();

    //sets function pointer from the DSO and the name of the function
    func = dlsym(descriptor, output_type_full);

    //checks error
    char* error = dlerror();
    if (error != NULL) {
        dlclose(descriptor);
        return 2;
    }

    //runs DSO function
    ssize_t ret = func(pixels, width, height, output_stream);

    //close everything
    //fclose(output_stream);
    dlclose(descriptor);

    return ret;
}
ssize_t plugin_write(const char *output_file,
                     const char *output_type,
                     uint16_t width,
                     uint16_t height,
                     const Pixel *pixels) 
{
    FILE * out = fopen(output_file, "w");
    return plugin_write_stream(out, output_type, width, height, pixels);  
}