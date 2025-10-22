// ray.h
// Defines a ray (point, direction).
// Stephen Marz
// 27-Aug-2025

#pragma once
#include "vec3.h"

struct Ray {
    Vec3 origin;
    Vec3 direction;
};

/**
 * Calculates where the ray would be at time t.
 * `const struct Ray *r` a ray with an origin and direction.
 * `double t` a time for the ray to move in the direction.
 */
Vec3 ray_at(const struct Ray *r, double t);

/**
 * Create a new ray from origin sent into the direction.
 */
struct Ray ray(Vec3 origin, Vec3 direction);

