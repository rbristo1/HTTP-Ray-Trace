#pragma once
#include "vec3.h"
#include "material.h"
#include <stdbool.h>

struct Ray;
struct Object;
struct Interval;
struct HitRecord;

typedef bool (*HITFUNC)(const struct Object *obj, 
                        const struct Ray *r,
                        const struct Interval *interval,
                        struct HitRecord *rec);

struct Object {
    struct Material material;
    Point           center;
    Vec3            normal;
    double          interval;
    HITFUNC         hit;
};
