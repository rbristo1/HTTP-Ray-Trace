#include "sphere.h"
#include "vec3.h"
#include "material.h"
#include <stdbool.h>
#include <math.h>
#include "ray.h"
#include "hit.h"
#include "interval.h"
bool sphere_hit(const struct Object *this, const struct Ray *r, const struct Interval *iv, struct HitRecord *result)
{
    const struct Sphere * sphere = (const struct Sphere *) this;
    double radius = sphere->radius;
    bool did_hit = false;
    Vec3 originToSphere = vec3_sub(r->origin, sphere->object.center);
    double a = pow(vec3_len(r->direction),2);
    double h = vec3_dot(r->direction, originToSphere);
    //double b = 2*h;
    double c = pow(vec3_len(originToSphere),2)-pow(radius, 2);
    double d = pow(h, 2) - a*c;
    if (d < 0) {
        return false;
    }
    double sqrtd = sqrt(d);
    //apparently h needs to be negative
    //this somehow fixed the cube.c error too????
    double test = (-h-sqrtd)/a;
    if (!interval_surrounds(iv, test)) {
        test = (-h+sqrtd)/a;
        if (!interval_surrounds(iv, test)) {
            return false;
        }
    }
    did_hit = true;
    result->t = test;
    result->material = sphere->object.material;
    Vec3 rayEnd = ray_at(r, test);
    result->point = rayEnd;
    Vec3 outward_normal = vec3_div_val(vec3_sub(rayEnd, sphere->object.center), sphere->radius);
    hit_record_face_normal(result, r, outward_normal);

    return did_hit;
}

struct Sphere sphere(Vec3 center, double radius, struct Material material)
{
    struct Sphere ret = {0};
    ret.radius = radius;
    ret.object.material = material;
    ret.object.center = center;
    ret.object.hit = sphere_hit;
    return ret;
}
