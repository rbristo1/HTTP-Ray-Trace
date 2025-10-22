// vec3.h
// Vector library
// Stephen Marz
// 12-August-2025

#pragma once

typedef struct Vec3 {
    union {
        struct {
            double x;
            double y;
            double z;
        };
        struct {
            double r;
            double g;
            double b;
        };
        double vec[3];
    };
} Vec3;

typedef Vec3 Color;
typedef Vec3 Pixel;
typedef Vec3 Point;

/**
 * Create a new Vec3 and set the x, y, and z
 * components to the given arguments, respectively.
 * @return a Vec3 with the initial values.
 */
Vec3 vec3(double x, double y, double z);
/**
 * Calculates the dot product of vectors `u` and `v`.
 * @return a double with the dot product calculation.
 */
double vec3_dot(Vec3 u, Vec3 v);
/**
 * Calculates the cross product of vectors `u` and `v`.
 * @return a new Vec3 with the cross product.
 */
Vec3 vec3_cross(Vec3 u, Vec3 v);
/**
 * Component multiplication of two vectors.
 * @return a new Vec3 with the multiplied components.
 */
Vec3 vec3_mul(Vec3 left, Vec3 right);
/**
 * Multiply the vector components with the given `val`.
 * @return a new Vec3 with the multiplied components.
 */
Vec3 vec3_mul_val(Vec3 left, double val);
/**
 * Component division of two vectors.
 * @return a new Vec3 with the division components.
 */
Vec3 vec3_div(Vec3 left, Vec3 right);
/**
 * Divide the vector components with the given `val`.
 * @return a new Vec3 with the divided components.
 */
Vec3 vec3_div_val(Vec3 left, double val);
/**
 * Component subtraction of two vectors.
 * @return a new Vec3 with the subtracted components.
 */
Vec3 vec3_sub(Vec3 left, Vec3 right);
/**
 * Subtract the vector components with the given `val`.
 * @return a new Vec3 with the subtracted components.
 */
Vec3 vec3_sub_val(Vec3 left, double val);
/**
 * Component addition of two vectors.
 * @return a new Vec3 with the added components.
 */
Vec3 vec3_add(Vec3 left, Vec3 right);
/**
 * Add the vector components with the given `val`.
 * @return a new Vec3 with the added components.
 */
Vec3 vec3_add_val(Vec3 left, double val);
/**
 * Calculates the unit length of the given vector.
 * @return the unit length as a `double`.
 */
double vec3_len(Vec3 v);
/**
 * Calculates the squared unit length of the given vector.
 * @return the squared unit length as a `double`.
 */
double vec3_len2(Vec3 v);
/**
 * Calculates the normal "unit" vector from `src`.
 * @return the unit vector from `src`.
 */
Vec3 vec3_unit(Vec3 src);

/**
 * HELPER MACROS
 */

/**
 * Create a vector with the same value, v, for all
 * three components.
 */
#define vec3_val(v) vec3((v), (v), (v))

/**
 * Create a vector with 0s as the initial value for all
 * three components.
 */
#define vec3_0    vec3_val(0)
#define vec3_1    vec3_val(1)

/**
 * Helper functions to quickly print the values of a `Vec3`
 */
#define vec3_print(v)     printf("{%.3lf, %.3lf, %.3lf}", (v).x, (v).y, (v).z)
#define vec3_println(v)   printf("{%.3lf, %.3lf, %.3lf}\n", (v).x, (v).y, (v).z)

/**
 * Helper to negate a vec3
 */
#define vec3_neg(v)     vec3_sub(vec3_0, (v))
