#include "scene.h"
#include "interval.h"
#include "object.h"
#include "hit.h"
#include <stdlib.h>
#include <math.h>

/*
struct Scene {
    Vec3 bg_color_1;
    Vec3 bg_color_2;
    unsigned int num_objects;
    struct Object **objects;
};
*/


/**
 * Create a new scene and return it. 
 * Both num_objects and objects will be zeroed.
 * The Scene pointer returned is freed by `scene_free`.
 */
struct Scene *scene_new(Vec3 bgcolor1, Vec3 bgcolor2) {
    struct Scene * scene = calloc(1, sizeof(struct Scene));
    scene->num_objects = 0;
    scene->objects = NULL;
    scene->bg_color_1 = bgcolor1;
    scene->bg_color_2 = bgcolor2;
    return scene;
}
/**
 * Clear all objects in the scene. This will free all
 * associated pointers to objects and zero out the scene.
 */
void scene_clear(struct Scene *scene) {
    /*for (int i = 0; i < scene->num_objects; i++) {
        if (scene->objects[i] != NULL) {
            free(scene->objects[i]]);
        }
    }*/
    free(scene->objects);
    scene->objects = NULL;
    scene->num_objects = 0;

}
/**
 * Add an object to the scene. This must be put in the order that
 * `scene_add` has been called.
 */
void scene_add(struct Scene *scene, struct Object *object) {
    if (scene->num_objects == 0) {
        scene->objects = calloc(1, sizeof(struct Object *));
        scene->objects[0] = object;
        scene->num_objects++;
    }
    else {
        scene->objects = realloc(scene->objects, (scene->num_objects+1)*sizeof(struct Object *));
        scene->objects[scene->num_objects] = object;
        scene->num_objects++;
    }
}
/**
 * Trace a ray at the given interval in this scene. The results
 * are stored in the HitRecord structure.
 * @param scene the scene to trace.
 * @param ray the ray to trace within the scene.
 * @param iv the interval of rays to be considered.
 * @param hr the hit record to store information of any object in the scene hit.
 * @return true if the ray hit any object in the scene, otherwise false.
 */
bool scene_trace(const struct Scene *scene, const struct Ray *ray, struct Interval *iv, struct HitRecord *hr) {
    //return false;
    bool hit = false;
    double closest = iv->max;
    bool hit2 = false;
    struct HitRecord temp;
    for (unsigned int i = 0; i < scene->num_objects; i++) {
        struct Interval inter = interval(iv->min, closest);
        
        hit2 = scene->objects[i]->hit(scene->objects[i], ray, &inter, &temp);
        if (hit2) {
            hit = true;
            closest = temp.t;
            *hr = temp;
            
        }
        
    }


    return hit;
}
/**
 * Free the scene given by scene. This does not free
 * individual objects, but only pointers to them.
 */
void scene_free(struct Scene *scene) {
    free(scene->objects);
    free(scene);
}