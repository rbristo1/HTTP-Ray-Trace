#pragma once
#include "vec3.h"
#include <stdbool.h>

struct Object;
struct Ray;
struct Interval;
struct HitRecord;
struct Camera;

struct Scene {
    Vec3 bg_color_1;
    Vec3 bg_color_2;
    unsigned int num_objects;
    struct Object **objects;
};

/**
 * Create a new scene and return it. 
 * Both num_objects and objects will be zeroed.
 * The Scene pointer returned is freed by `scene_free`.
 */
struct Scene *scene_new(Vec3 bgcolor1, Vec3 bgcolor2);
/**
 * Clear all objects in the scene. This will free all
 * associated pointers to objects and zero out the scene.
 */
void scene_clear(struct Scene *scene);
/**
 * Add an object to the scene. This must be put in the order that
 * `scene_add` has been called.
 */
void scene_add(struct Scene *scene, struct Object *object);
/**
 * Trace a ray at the given interval in this scene. The results
 * are stored in the HitRecord structure.
 * @param scene the scene to trace.
 * @param ray the ray to trace within the scene.
 * @param iv the interval of rays to be considered.
 * @param hr the hit record to store information of any object in the scene hit.
 * @return true if the ray hit any object in the scene, otherwise false.
 */
bool scene_trace(const struct Scene *scene, const struct Ray *ray, struct Interval *iv, struct HitRecord *hr);
/**
 * Free the scene given by scene. This does not free
 * individual objects, but only pointers to them.
 */
void scene_free(struct Scene *scene);
/**
 * Read a scene from a file.
 * @param file the scene input file.
 * @param scene the scene with the new values.
 * @param camera the camera loaded the given scene.
 * @return true if successful, false otherwise.
 */
bool scene_read(const char *file, struct Scene *scene, struct Camera *camera);
