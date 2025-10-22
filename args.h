#pragma once

#include <stdio.h>

typedef struct Options {
    char output_type[4];   // the type of output: (bmp, rto, or ppm)
    char output_file[256]; // the file name or - for stdout
    int width;             // width of picture
    int height;            // height of picture
    int seed;              // rng seed
    int threads;           // number of threads
} Options;

typedef enum OptionsType {
    OT_NONE,
    OT_OUTPUT_TYPE,
    OT_OUTPUT_FILE,
    OT_WIDTH,
    OT_HEIGHT,
    OT_SEED,
    OT_THREADS,
} OptionsType;

typedef enum OptionsError {
    OE_SUCCESS,
    OE_SWITCH_REQUIRES_PARAMETER,
    OE_INVALID_VALUE_FOR_PARAMETER,
    OE_UNKNOWN_SWITCH
} OptionsError;

typedef struct OptionsResult {
    int argno;
    OptionsType otype;
    OptionsError oerror;
} OptionsResult;

// Range of arguments
#define MIN_WIDTH 50
#define MAX_WIDTH 9000
#define MIN_HEIGHT 50
#define MAX_HEIGHT 9000
#define MIN_SEED 0
#define MAX_SEED 10000
#define MIN_THREADS 1
#define MAX_THREADS 400

/**
 * Parse the arguments and write the argument values in `options`.
 * @param options   a pointer to the options structure to fill.
 * @param count     the number of *user-supplied* arguments.
 * @param arguments the *user-supplied* arguments.
 * @return an `OptionsResult`
 */
OptionsResult args_parse(Options *options, int count, const char *arguments[]);
