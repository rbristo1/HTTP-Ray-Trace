#pragma once
#include "vec3.h"
#include "material.h"
#include <stdbool.h>

struct Ray;
struct HitRecord {
    Point            point;
    Vec3             normal;
    struct Material  material;
    double           t;
    bool             front_face;
};

void hit_record_face_normal(struct HitRecord *hr, const struct Ray *r, Vec3 outward_normal);
struct HitRecord hit_record(Point p, Vec3 normal, struct Material mat, double t);
