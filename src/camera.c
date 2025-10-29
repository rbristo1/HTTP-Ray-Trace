#include "camera.h"
#include <math.h>
#include "ray.h"
#include "vec3.h"
#include "scene.h"
#include "camera.h"
#include "interval.h"
#include "hit.h"
#include <stdlib.h>
#include <pthread.h>

static double deg2rad(double deg)
{
    return M_PI * deg / 180.0;
}


struct Data {
    pthread_t tid;
    int rows;
    int offset;
    const struct Camera *camera;
    const struct Scene *scene;
    Pixel * pixels;
};
void *worker(void *arg)
{
    struct Data *data = (struct Data *)arg;
    ssize_t totalPixels = data->rows*data->camera->image_width;
    for (ssize_t i = data->offset; i < data->offset+data->rows; i++) {
        for (ssize_t j = 0; j < data->camera->image_width; j++) {
            if (data->camera->samples_per_pixel > 0) {
                Pixel temp = {0,0,0};
                for (ssize_t k = 0; k < data->camera->samples_per_pixel; k++) {
                    struct Ray r = get_ray_ms(data->camera, i, j, k);
                    temp = vec3_add(temp, ray_color(&r, data->camera->max_depth, data->scene));
                }
                data->pixels[i*data->camera->image_width+j] = vec3_div_val(temp, data->camera->samples_per_pixel);
            }
            else {
                struct Ray r = get_ray(data->camera, i, j);
                data->pixels[i*data->camera->image_width+j] = ray_color(&r, data->camera->max_depth, data->scene);
            }
            
        }
    }
    
    return NULL;
}

struct Ray get_ray(const struct Camera *camera, int row, int col) {
    Vec3 off_u = vec3_mul_val(camera->pixel_delta_u, (double)col);
    Vec3 off_v = vec3_mul_val(camera->pixel_delta_v, (double)row);
    
    /*Point pixel_point = vec3_add(vec3_add(camera->pixel00_location, off_u), off_v);
     

    Vec3 dir = vec3_sub(pixel_point, camera->look_from);
    return ray(camera->look_from, dir);*/
    Vec3 dir = vec3_add(vec3_add(off_u, camera->pixel00_location), off_v);
    return ray(camera->look_from, dir);
}

struct Ray get_ray_ms(const struct Camera *camera, int row, int col, int sample) {
    if (camera->sampling_strategy == SAMP_LINEAR_X) {
        double delta = (double)sample / camera->samples_per_pixel;
        col += delta;
    }
    else if (camera->sampling_strategy == SAMP_LINEAR_Y) {
        double delta = (double)sample / camera->samples_per_pixel;
        row += delta;
    }
    else {
        double side = sqrt(camera->samples_per_pixel);
        col += sample / side;
        row += fmod(sample, side);
    }
    Vec3 off_u = vec3_mul_val(camera->pixel_delta_u, (double)col);
    Vec3 off_v = vec3_mul_val(camera->pixel_delta_v, (double)row);
    //Point pixel_point = vec3_add(vec3_add(camera->pixel00_location, off_u), off_v);

    
    Vec3 dir = vec3_add(vec3_add(off_u, camera->pixel00_location), off_v);
    return ray(camera->look_from, dir);
}

Color ray_color(const struct Ray *r, int depth, const struct Scene *scene) {
    if (depth <= 0) {
        return vec3(0,0,0);
    }
    struct Interval iv = interval(.001, INFINITY);
    struct HitRecord hr;
    hr.t = -1.0;
    scene_trace(scene, r, &iv, &hr);
    if (hr.t != -1.0) {
        struct Ray scatter;
        Color att;
        if (material_scatter(&hr.material, &hr, r, &att, &scatter)) {
            return vec3_mul(att, ray_color(&scatter, depth-1, scene));
        }
        return vec3(0,0,0);
    }
    else {
        Vec3 direction = vec3_unit(r->direction);
        double alpha = (direction.y+1.0)/2.0;
        return lerp(alpha, scene->bg_color_1, scene->bg_color_2);
    }
}
/*
struct Camera {
    typedef enum {
        SAMP_LINEAR_X,
        SAMP_LINEAR_Y,
        SAMP_BOX
    } SamplingStrategy;
    int     samples_per_pixel;
    SamplingStrategy sampling_strategy;
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
*/
/**
 * Create a new camera given image_height, image_width, and center point.
 * Most of the other fields are calculated within the construction function.
 */
 
struct Camera camera(int image_width, int image_height)
{
    
    struct Camera ret = {0};
    ret.samples_per_pixel = 0;
    ret.sampling_strategy = SAMP_LINEAR_X;
    ret.image_height = image_height;
    ret.image_width = image_width;
    ret.max_depth = 10;
    ret.vfov = 90.0;
    ret.focus_dist = 10;
    ret.look_from = vec3(0,0,0);
    ret.look_at = vec3(0,0,-1);
    ret.v_up = vec3(0,1,0);
    return ret;
}
/**
 * Calculate non-default values
 */
void camera_init(struct Camera *camera) {
    double vfovAngle = deg2rad(camera->vfov);
    double viewportHeight = 2.0*tan(vfovAngle/2.0)*camera->focus_dist;
    double viewportWidth = viewportHeight*((double)(camera->image_width)/(double)(camera->image_height));
    camera->w = vec3_unit(vec3_sub(camera->look_from, camera->look_at));
    camera->u = vec3_unit(vec3_cross(camera->v_up, camera->w));
    camera->v = vec3_unit(vec3_cross(camera->w, camera->u));
    Vec3 Vu = vec3_mul_val(camera->u, viewportWidth); 
    Vec3 Vv = vec3_mul_val(vec3_mul_val(camera->v, -1.0), viewportHeight);
    camera->pixel_delta_u = vec3_div_val(Vu, camera->image_width);
    camera->pixel_delta_v = vec3_div_val(Vv, camera->image_height);
    Point viewport_upper_left = vec3_sub(vec3_sub(vec3_sub(camera->look_from, vec3_mul_val(camera->w, camera->focus_dist)), vec3_div_val(Vu, 2.0)), vec3_div_val(Vv, 2.0));
    camera->pixel00_location = vec3_add(vec3_div_val(vec3_add(camera->pixel_delta_u, camera->pixel_delta_v), 2.0), viewport_upper_left);

}
/**
 * Render a scene with the given camera. Returns the array of RGB pixels.
 * There will be camera->image_height * camera->image_width number of pixels.
 * Returns NULL if the scene/camera is invalid.
 */
Pixel *camera_render(const struct Camera *camera, const struct Scene *scene)
{
    
    ssize_t totalPixels = camera->image_height*camera->image_width;
    Pixel *pixels = calloc(totalPixels, sizeof(Pixel));
    for (ssize_t i = 0; i < camera->image_height; i++) {
        for (ssize_t j = 0; j < camera->image_width; j++) {
            if (camera->samples_per_pixel > 0) {
                Pixel temp = {0,0,0};
                for (ssize_t k = 0; k < camera->samples_per_pixel; k++) {
                    struct Ray r = get_ray_ms(camera, i, j, k);
                    temp = vec3_add(temp, ray_color(&r, camera->max_depth, scene));
                }
                pixels[i*camera->image_width+j] = vec3_div_val(temp, camera->samples_per_pixel);
            }
            else {
                struct Ray r = get_ray(camera, i, j);
                pixels[i*camera->image_width+j] = ray_color(&r, camera->max_depth, scene);
            }
            
            
        }
    }

    return pixels;
}

/**
 * Multi-threaded render a scene with the given camera.
 * Returns the array of RGB pixels.
 * num_threads controls the number of threads that will trace in parallel.
 * There will be camera->image_height * camera->image_width number of pixels.
 * Returns NULL if the scene/camera is invalid.
 */
Pixel *camera_mt_render(const struct Camera *camera, const struct Scene *scene, int num_threads) {
    if (!camera || !scene) {
        return NULL;
    }
    pthread_cond_t complete = PTHREAD_COND_INITIALIZER;
    int num_rows_to_work_on = camera->image_height / num_threads;
    int excess_rows = camera->image_height % num_threads;
    struct Data data[num_threads];
    ssize_t totalPixels = camera->image_height*camera->image_width;
    Pixel * temp = calloc(totalPixels, sizeof(Pixel));
    for (int i = 0; i < num_threads; i++) {
        if (i < excess_rows) {
            data[i].rows = num_rows_to_work_on+1;
        }
        else {
            data[i].rows = num_rows_to_work_on;
        }
        if (i < excess_rows) {
            data[i].offset = i*(num_rows_to_work_on+1);
        }
        else {
            data[i].offset = excess_rows*(num_rows_to_work_on+1);
        }
        if (i > excess_rows) {
            data[i].offset += (i-excess_rows)*num_rows_to_work_on;
        }
        data[i].camera = camera;
        data[i].scene = scene;
        data[i].pixels = temp;
        pthread_create(&data[i].tid, NULL, worker, data + i);

    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(data[i].tid, NULL);
    }

    return temp;
}
/**
 * Linear Interpolation (LERP). 
 * The alpha is 0.0 - 1.0. Values outside of this range
 * is undefined behavior.
 * 0.0 is 100% of `c1 and 0% of `c2`.
 * 0.5 is a 50/50 mixture of `c1` and `c2`.
 * 1.0 is 0% `c1` and 100% of `c2`.
 */
Vec3 lerp(double alpha, Vec3 c1, Vec3 c2)
{
    return vec3_add(vec3_mul_val(c1, alpha), vec3_mul_val(c2, 1.0-alpha));
}
