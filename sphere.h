#pragma once
#include "object.h"
#include "vec3.h"
#include "material.h"
#include <stdbool.h>

struct Sphere {
    struct Object object;
    double radius;
};

struct Sphere sphere(Vec3 center, double radius, struct Material material);
