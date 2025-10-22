#include "material.h"
#include "scene.h"
#include "cube.h"
#include "sphere.h"
#include "camera.h"
#include "plugin.h"
#include <stdlib.h>
#include <stdio.h>

#define WIDTH 7680
#define HEIGHT 4320
int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    struct Scene *scene;
    struct Camera cam;
    struct Sphere s[5];
    struct Cube c[5];
    int num_spheres = 0;
    int num_cubes = 0;
    int i;
    Pixel *pixels;

    // New scene with a purple-to-white gradient along Y.
    scene = scene_new(vec3(0.57, 0.2, 0.6),
                      vec3(1, 1, 1));

    // Gray sphere (upper-right)
    s[num_spheres++] = sphere(vec3(25, 10, -100),
                              10,
                              material_lambertian(vec3(0.5, 0.5, 0.5)));
    // Red sphere (upper-left)
    s[num_spheres++] = sphere(vec3(-10, 3, -30),
                              5,
                              material_metal(vec3(1.0, 0.0, 0.0), 0.77));
    // Glass sphere (upper-middle)
    s[num_spheres++] = sphere(vec3(1, 2, -20),
                              2,
                              material_dielectric( 0.77));

    for (i = 0;i < num_spheres;i+=1) {
        scene_add(scene, &s[i].object);
    }

    // Dark blue cube (bottom-middle)
    c[num_cubes++] = cube(vec3(0, -45, -120),
                          25.5, 12.2, 87.65,
                          material_lambertian(vec3(0.0, 0.7, 0.9)));
    // Light blue cube (bottom-left)
    c[num_cubes++] = cube(vec3(-45, -45, -220),
                          15.5, 52.2, 17.65,
                          material_metal(vec3(0.0, 0.7, 0.9), 0.13));
    // Glass cube (middle-middle)
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
    cam.focus_dist = 10.0;

    camera_init(&cam);

    pixels = camera_mt_render(&cam, scene, 16);

    // Scene has been rendered! Output it as a BMP file.
    
    plugin_write(argv[1],
                 "bmp",
                 cam.image_width,
                 cam.image_height,
                 pixels);

    scene_free(scene);
    free(pixels);
    return 0;
}
