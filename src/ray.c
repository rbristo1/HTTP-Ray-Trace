#include "ray.h"

// Write your ray utilities here.

// Calculates where the ray would be at time t.
Vec3 ray_at(const struct Ray *r, double t) {
    return vec3_add(r->origin, vec3_mul_val(r->direction, t));
}

// Create a new ray from origin sent into the direction.
struct Ray ray(Vec3 origin, Vec3 direction) {
    struct Ray newRay;
    newRay.origin = origin;
    newRay.direction = direction;
    return newRay;
}