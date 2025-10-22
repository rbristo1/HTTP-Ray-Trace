#pragma once
#include "object.h"
#include "material.h"
#include "vec3.h"
#include <stdbool.h>

struct Cube {
    struct Object object;
    double width;
    double height;
    double depth;
};

struct Cube cube(Vec3 center, double width, double height, double depth, struct Material material);
