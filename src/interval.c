#include "interval.h"
#include <float.h>

/**
 * Default interval from -DBL_MAX..DBL_MAX.
 */
struct Interval interval_default(void) {
    struct Interval def;
    def.min = -DBL_MAX;
    def.max = DBL_MAX;
    return def;
}
/**
 * New interval from mn..mx.
 */
struct Interval interval(double mn, double mx){
    struct Interval def;
    def.min = mn;
    def.max = mx;
    return def;
}
/**
 * Calculate the distance in the interval (max - min).
 */
double interval_size(const struct Interval *iv) {
    return iv->max-iv->min;
}
/**
 * Returns true if `x` is within the inclusive boundaries of
 * the interval `iv`.
 */
bool interval_contains(const struct Interval *iv, double x) {
    if (x >= iv->min && x <= iv->max) {
        return true;
    }
    return false;
}
/**
 * Returns true if `x` is within the exclusive boundaries of
 * the interval `iv`.
 */
bool interval_surrounds(const struct Interval *iv, double x) {
    if (x > iv->min && x < iv->max) {
        return true;
    }
    return false;
}
/**
 * Clamps an interval to x.
 */
double interval_clamp(const struct Interval *iv, double x) {
    if (x < iv->min) {
        return iv->min;
    }
    else if (x > iv->max) {
        return iv->max;
    }
    else {
        return x;
    }
}

