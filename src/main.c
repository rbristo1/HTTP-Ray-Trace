#include "material.h"
#include "scene.h"
#include "cube.h"
#include "sphere.h"
#include "camera.h"
#include "plugin.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MT 16
#define WIDTH 3840
#define HEIGHT 2160
#define SAMPLES 36
#define SEED time(NULL)

#define SAMPLING_STRATEGY SAMP_BOX


int main(int argc, char *argv[])
{
    struct Scene *scene;
    struct Camera cam;

    if (argc < 2) {
        printf("Usage: %s <output>\n", argv[0]);
        return 1;
    }

    Pixel *pixels;
    int num_spheres = 0;
    int num_cubes = 0;
    struct Cube c[5];
    struct Sphere s[5];
    int i;

    srand(SEED);
    scene = scene_new(vec3(0.57, 0.2, 0.6),
                      vec3(1, 1, 1));

    s[num_spheres++] = sphere(vec3(25, 10, -100),
                              10,
                              material_lambertian(vec3(0.5, 0.5, 0.5)));

    s[num_spheres++] = sphere(vec3(-10, 3, -30),
                              5,
                              material_metal(vec3(1.0, 0.0, 0.0), 0.77));

    s[num_spheres++] = sphere(vec3(1, 2, -20),
                              2,
                              material_dielectric( 0.77));

    for (i = 0;i < num_spheres;i+=1) {
        scene_add(scene, &s[i].object);
    }

    c[num_cubes++] = cube(vec3(0, -45, -120),
                          25.5, 12.2, 87.65,
                          material_lambertian(vec3(0.0, 0.7, 0.9)));

    c[num_cubes++] = cube(vec3(-45, -45, -220),
                          15.5, 52.2, 17.65,
                          material_metal(vec3(0.0, 0.7, 0.9), 0.13));

    c[num_cubes++] = cube(vec3(1, -5, -60),
                          15.5, 12.2, 17.65,
                          material_dielectric(0.53));

    for (i = 0;i < num_cubes;i+=1) {
        scene_add(scene, &c[i].object);
    }

    cam = camera(WIDTH, HEIGHT);

    cam.max_depth = 20;
    cam.vfov = 55;
    cam.look_from = vec3(0, 0, 0);
    cam.look_at = vec3(0, 0, -1);
    cam.v_up = vec3(0, 1, 0);
    cam.samples_per_pixel = SAMPLES;
    cam.sampling_strategy = SAMPLING_STRATEGY;
    cam.focus_dist = 10.0;

    camera_init(&cam);
    #ifdef MT
        pixels = camera_mt_render(&cam, scene, MT);
        printf("used mt\n");
    #else
        pixels = camera_render(&cam, scene);
        printf("used reg\n");
    #endif

    if (NULL != pixels) {
        plugin_write(argv[1], "bmp",
                     cam.image_width, cam.image_height,
                     pixels);
    }
    else {
        printf("camera_render returned NULL!\n");
    }

    scene_free(scene);
    free(pixels);
    return 0;
}