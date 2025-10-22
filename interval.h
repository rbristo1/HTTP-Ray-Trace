#pragma once
#include <stdbool.h>

struct Interval {
    double min;
    double max;
};

/**
 * Default interval from -INFINITY..INFINITY.
 */
struct Interval interval_default(void);
/**
 * New interval from mn..mx.
 */
struct Interval interval(double mn, double mx);
/**
 * Calculate the distance in the interval (max - min).
 */
double interval_size(const struct Interval *iv);
/**
 * Returns true if `x` is within the inclusive boundaries of
 * the interval `iv`.
 */
bool interval_contains(const struct Interval *iv, double x);
/**
 * Returns true if `x` is within the exclusive boundaries of
 * the interval `iv`.
 */
bool interval_surrounds(const struct Interval *iv, double x);
/**
 * Clamps an interval to x.
 */
double interval_clamp(const struct Interval *iv, double x);
