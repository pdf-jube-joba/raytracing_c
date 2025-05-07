#ifndef VEC3_H
#define VEC3_H

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <limits.h> // UINT_MAX

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

// ====== vector ======

// type for 3 set of float
typedef struct
{
    double x, y, z;
} vec3;

static inline vec3 vec3_make(double x, double y, double z)
{
    vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

static inline vec3 vec3_add(vec3 a, vec3 b)
{
    return vec3_make(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline vec3 vec3_inv(vec3 a)
{
    return vec3_make(-a.x, -a.y, -a.z);
}

static inline vec3 vec3_sub(vec3 a, vec3 b)
{
    return vec3_make(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline vec3 vec3_scale(vec3 a, double s)
{
    return vec3_make(a.x * s, a.y * s, a.z * s);
}

static inline double vec3_dot(vec3 a, vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline double vec3_length(vec3 a)
{
    return sqrt(vec3_dot(a, a));
}

static inline vec3 vec3_unit(vec3 a)
{
    double len = vec3_length(a);
    if (len == 0.0)
    {
        return vec3_make(0.0, 0.0, 0.0);
    }
    return vec3_scale(a, 1.0 / len);
}

static inline vec3 vec3_cross(vec3 a, vec3 b)
{
    return vec3_make(a.y * b.z - a.z * b.y,
                     a.z * b.x - a.x * b.z,
                     a.x * b.y - a.y * b.x);
}

static inline vec3 vec3_reflect(vec3 v, vec3 n)
{
    return vec3_sub(v, vec3_scale(n, 2.0 * vec3_dot(v, n)));
}

// // cos^3 phi なのでだめらしい
// static inline vec3 random_unit_vector(unsigned int *state)
// {
//     vec3 random = vec3_make(
//         rand_unit(state) * 2.0 - 1.0,
//         rand_unit(state) * 2.0 - 1.0,
//         rand_unit(state) * 2.0 - 1.0);
//     return vec3_unit(random);
// }

static inline vec3 random_unit_vector(unsigned int *state)
{
    double a = rand_range(state, 0, 2 * MY_PI);
    double z = rand_range(state, -1, 1);
    double r = sqrt(1 - z * z);
    return vec3_make(r * cos(a), r * sin(a), z);
}

static inline vec3 refract(vec3 uv, vec3 n, double etai_over_etat)
{
    double cos_theta = vec3_dot(vec3_inv(uv), n);
    vec3 r_out_parallel = vec3_scale(vec3_add(uv, vec3_scale(n, cos_theta)), etai_over_etat);
    vec3 r_out_perp = vec3_scale(n, -sqrt(1.0 - vec3_dot(r_out_parallel, r_out_parallel)));
    return vec3_add(r_out_perp, r_out_parallel);
}

// type for point
typedef vec3 point;

typedef struct
{
    point origin;   // origin of ray
    vec3 direction; // direction of ray
} ray;

static inline ray ray_make(point origin, vec3 direction)
{
    assert(vec3_length(direction) > 0.0);
    ray r;
    r.origin = origin;
    r.direction = direction;
    return r;
}

static inline point ray_at(ray r, double t)
{
    return vec3_add(r.origin, vec3_scale(r.direction, t));
}

// type for color
// 0 <= x, y, z <= 1
// x for red, y for green, z for blue
typedef vec3 color;

static inline color color_make(double x, double y, double z)
{
    assert(x >= 0.0 && x <= 1.0);
    assert(y >= 0.0 && y <= 1.0);
    assert(z >= 0.0 && z <= 1.0);
    return vec3_make(x, y, z);
}

// a * (1 - t) + b * t
static inline color color_mix(color a, color b, double t)
{
    assert(t >= 0.0 && t <= 1.0);
    color at = vec3_scale(a, 1.0 - t);
    color bt = vec3_scale(b, t);
    return vec3_add(at, bt);
}

static inline color color_attenuation(color a, color b)
{
    return color_make(
        a.x * b.x,
        a.y * b.y,
        a.z * b.z);
}

static inline color background_color(ray r)
{
    vec3 unit_direction = vec3_unit(r.direction);
    double t = 0.5 * (unit_direction.y + 1.0);
    return color_mix(
        color_make(1.0, 1.0, 1.0),
        color_make(0.5, 0.7, 1.0),
        t);
}

void write_color(FILE *f, color c)
{
    fprintf(f, "%d %d %d\n",
            (unsigned char)(255.999 * c.x),
            (unsigned char)(255.999 * c.y),
            (unsigned char)(255.999 * c.z));
}

void write_color_gamma(FILE *f, color c)
{
    fprintf(f, "%d %d %d\n",
            (unsigned char)(255.999 * sqrt(c.x)),
            (unsigned char)(255.999 * sqrt(c.y)),
            (unsigned char)(255.999 * sqrt(c.z)));
}

#endif
