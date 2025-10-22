#include "cube.h"
#include "object.h"
#include "sphere.h"
#include "vec3.h"
#include "material.h"
#include <stdbool.h>
#include <math.h>
#include "ray.h"
#include "hit.h"
#include "interval.h"
#include <stdlib.h>
bool cube_hit(const struct Object *this, const struct Ray *r, const struct Interval *iv, struct HitRecord *result)
{
    bool did_hit = false;
    const struct Cube * cube = (const struct Cube *) this;
    double x = cube->width/2;
    double y = cube->height/2;
    double z = cube->depth/2;
    Vec3 halfCube = vec3(x,y,z);
    Vec3 min_bound = vec3_sub(cube->object.center, halfCube);
    Vec3 max_bound = vec3_add(cube->object.center, halfCube);
    double tmin = iv->min;
    double tmax = iv->max;
    for (int i = 0; i < 3; i++) {
        double origin, minBound, maxBound, direction;
        if (i == 0) {
            origin = r->origin.x;
            minBound = min_bound.x;
            maxBound = max_bound.x;
            direction = r->direction.x;
        }
        else if (i == 1) {
            origin = r->origin.y;
            minBound = min_bound.y;
            maxBound = max_bound.y;
            direction = r->direction.y;
        }
        else {
            origin = r->origin.z;
            minBound = min_bound.z;
            maxBound = max_bound.z;
            direction = r->direction.z;
        }

        
        double t1, t0;
        if (direction < 0) {
            t1 = ((minBound-origin)/direction);
            t0 = ((maxBound-origin)/direction);
        }
        else{
            t0 = ((minBound-origin)/direction);
            t1 = ((maxBound-origin)/direction);
        }
        /*else {
            t0 = tmin;
            t1 = tmax;
        }*/
        /*if (i == 0) {
            tmin = t0;
            tmax = t1;
        }
        else {*/
            if (t0 > tmin) {
                tmin = t0;
            }
            if (t1 < tmax) {
                tmax = t1;
            }
            if (tmax <= tmin) {
                return false;
            }
        //}
    }  
    
    if (interval_surrounds(iv, tmin)) {
        did_hit = true;
    }
    else {
        return false;
    }
    result->t = tmin;
    result->point = ray_at(r, tmin);
    Vec3 outwardNormal = vec3(0,0,0);
    for (int i = 0; i < 3; i++) {
        double minBound, maxBound, point;
        if (i == 0) {
            minBound = min_bound.x;
            maxBound = max_bound.x;
            point = result->point.x;
        }
        else if (i == 1) {
            minBound = min_bound.y;
            maxBound = max_bound.y;
            point = result->point.y;
        }
        else {
            minBound = min_bound.z;
            maxBound = max_bound.z;
            point = result->point.z;
        }
        double absDiff = fabs(point-minBound);
        //double absDiff = point-minBound;
        double absDiff2 = fabs(point-maxBound);
        //double absDiff2 = point-maxBound;
        if (i == 0) {
            if (absDiff < 1*pow(10,-8)) {
                outwardNormal.x = -1.0;
            }
            else if (absDiff2 < 1*pow(10,-8)) {
                outwardNormal.x = 1.0;
            }
            else {
                outwardNormal.x = 0.0;
            }
        }
        else if (i == 1) {
            if (absDiff < 1*pow(10,-8)) {
                outwardNormal.y = -1.0;
            }
            else if (absDiff2 < 1*pow(10,-8)) {
                outwardNormal.y = 1.0;
            }
            else {
                outwardNormal.y = 0.0;
            }
        }
        else if (i == 2) {
            if (absDiff < 1*pow(10,-8)) {
                outwardNormal.z = -1.0;
            }
            else if (absDiff2 < 1*pow(10,-8)) {
                outwardNormal.z = 1.0;
            }
            else {
                outwardNormal.z = 0.0;
            }
        }
    }
    
    hit_record_face_normal(result, r, outwardNormal);
    result->material = cube->object.material;
    return did_hit;
}

struct Cube cube(Vec3 center, double width, double height, double depth, struct Material material)
{
    struct Cube ret = {0};
    ret.width = width;
    ret.height = height;
    ret.depth = depth;
    ret.object.material = material;
    ret.object.center = center;
    ret.object.hit = cube_hit;

    return ret;
}