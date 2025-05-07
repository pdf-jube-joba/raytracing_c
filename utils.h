#ifndef UTILS_H
#define UTILS_H

#include <sys/time.h> // for time
#include <math.h>
#include <limits.h> // UINT_MAX

// pi 
#define MY_PI 3.14159265358979323846

// ====== randoms ======
static inline double xor_shift(unsigned int *state)
{
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}

// return [0, 1)
static inline double rand_unit(unsigned int *state)
{
    return xor_shift(state) / (double)UINT_MAX;
}

static inline double rand_range(unsigned int *state, double min, double max)
{
    return min + (max - min) * rand_unit(state);
}

// ====== utility ======

static double schlick(double cosine, double ref_idx)
{
    double r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

// ====== time ======
double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec - st.tv_sec) + (et.tv_usec - st.tv_usec) / 1000000.0;
}

#endif
