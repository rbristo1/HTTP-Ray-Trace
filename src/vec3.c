#include "vec3.h"
#include <math.h>
// Write all of your vec3 functions here.




/**
 * Create a new Vec3 and set the x, y, and z
 * components to the given arguments, respectively.
 * @return a Vec3 with the initial values.
 */
Vec3 vec3(double x, double y, double z) {
    return (Vec3){.x = x, .y = y, .z = z};
}
/**
 * Calculates the dot product of vectors `u` and `v`.
 * @return a double with the dot product calculation.
 */
double vec3_dot(Vec3 u, Vec3 v) {
    return (u.x * v.x + u.y * v.y + u.z * v.z);
}
/**
 * Calculates the cross product of vectors `u` and `v`.
 * @return a new Vec3 with the cross product.
 */
Vec3 vec3_cross(Vec3 u, Vec3 v) {
    return vec3(u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}
/**
 * Component multiplication of two vectors.
 * @return a new Vec3 with the multiplied components.
 */
Vec3 vec3_mul(Vec3 left, Vec3 right) {
    return vec3(left.x*right.x, left.y*right.y, left.z*right.z);
}
/**
 * Multiply the vector components with the given `val`.
 * @return a new Vec3 with the multiplied components.
 */
Vec3 vec3_mul_val(Vec3 left, double val) {
    return vec3(left.x*val, left.y*val, left.z*val);
}
/**
 * Component division of two vectors.
 * @return a new Vec3 with the division components.
 */
Vec3 vec3_div(Vec3 left, Vec3 right) {
    return vec3(left.x/right.x, left.y/right.y, left.z/right.z);
}
/**
 * Divide the vector components with the given `val`.
 * @return a new Vec3 with the divided components.
 */
Vec3 vec3_div_val(Vec3 left, double val) {
    return vec3(left.x/val, left.y/val, left.z/val);
}
/**
 * Component subtraction of two vectors.
 * @return a new Vec3 with the subtracted components.
 */
Vec3 vec3_sub(Vec3 left, Vec3 right) {
    return vec3(left.x-right.x, left.y-right.y, left.z-right.z);
}
/**
 * Subtract the vector components with the given `val`.
 * @return a new Vec3 with the subtracted components.
 */
Vec3 vec3_sub_val(Vec3 left, double val) {
    return vec3(left.x-val, left.y-val, left.z-val);
}
/**
 * Component addition of two vectors.
 * @return a new Vec3 with the added components.
 */
Vec3 vec3_add(Vec3 left, Vec3 right) {
    return vec3(left.x+right.x, left.y+right.y, left.z+right.z);
}
/**
 * Add the vector components with the given `val`.
 * @return a new Vec3 with the added components.
 */
Vec3 vec3_add_val(Vec3 left, double val) {
    return vec3(left.x+val, left.y+val, left.z+val);
}
/**
 * Calculates the unit length of the given vector.
 * @return the unit length as a `double`.
 */
double vec3_len(Vec3 v) {
    return sqrt(vec3_len2(v));
}
/**
 * Calculates the squared unit length of the given vector.
 * @return the squared unit length as a `double`.
 */
double vec3_len2(Vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
/**
 * Calculates the normal "unit" vector from `src`.
 * @return the unit vector from `src`.
 */
Vec3 vec3_unit(Vec3 src) {
    double len = vec3_len(src);
    return vec3(src.x / len, src.y / len, src.z / len);
}


