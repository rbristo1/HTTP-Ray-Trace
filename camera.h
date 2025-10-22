#pragma once
#include "vec3.h"
#include <stdio.h>


struct Scene;
struct Ray;

struct Camera {
    int     image_height;
    int     image_width;
    int     max_depth;
    double  vfov;
    double  focus_dist;
    Point   look_from;
    Point   look_at;
    Point   pixel00_location;
    Vec3    v_up;
    Vec3    pixel_delta_u;
    Vec3    pixel_delta_v;
    Vec3    u;
    Vec3    v;
    Vec3    w;
};

/**
 * Create a new camera given image_height and image_width.
 * Most of the other fields are calculated within the construction function.
 */
struct Camera camera(int image_width, int image_height);
/**
 * Create a new camera given image_height and image_width.
 * Most of the other fields are calculated within the construction function.
 */
Pixel *camera_mt_render(const struct Camera *camera, const struct Scene *scene, int num_threads);
void camera_init(struct Camera *camera);
/**
 * Render a scene with the given camera. Returns the array of RGB pixels.
 * There will be camera->image_height * camera->image_width number of pixels.
 * Returns NULL if the scene/camera is invalid.
 */
Pixel *camera_render(const struct Camera *camera, const struct Scene *scene);
/**
 * Linear Interpolation (LERP). 
 * The alpha is 0.0 - 1.0. Values outside of this range
 * is undefined behavior.
 * 0.0 is 100% of `c1 and 0% of `c2`.
 * 0.5 is a 50/50 mixture of `c1` and `c2`.
 * 1.0 is 0% `c1` and 100% of `c2`.
 */
Color lerp(double alpha, Color c1, Color c2);

struct Ray get_ray(const struct Camera *camera, int row, int col);
Color ray_color(const struct Ray *r, int depth, const struct Scene *scene);
