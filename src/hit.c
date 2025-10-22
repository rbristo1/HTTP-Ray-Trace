#include "hit.h"
#include "vec3.h"
#include "ray.h"
#include "material.h"
#include <stdbool.h>

void hit_record_face_normal(struct HitRecord *hr, const struct Ray *r, Vec3 outward_normal)
{
    double normal = vec3_dot(r->direction, outward_normal);
    double direction = -1.0;
    if (normal < 0) {
        direction = 1.0;
    }
    hr->normal = vec3_mul_val(outward_normal, direction);
    if (normal < 0) {
        hr->front_face = true;
    }
    else {
        hr->front_face = false;
    }
}

struct HitRecord hit_record(Point p, Vec3 normal, struct Material mat, double t)
{
    struct HitRecord hr;
    hr.point = p;
    hr.normal = normal;
    hr.material = mat;
    hr.t = t;
    return hr;
}
