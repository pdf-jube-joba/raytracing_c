#ifndef SCENE
#define SCENE

// world_entity_comb を使ったシーンの作成

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "vec3.h"
#include "world_entity_comb.h"

#define MY_PI 3.14159265358979323846

#define WIDTH 512
#define HEIGHT 512

static vec3 CAMERA_ORIGIN;
static vec3 HORIZONTAL;
static vec3 VERTICAL;
static vec3 LOWER_LEFT_CORNER;

static sphere_metal *SPHERES_METAL;
static sphere_lambertian *SPHERES_LAMBERTIAN;
static triangle_metal *TRIANGLES_METAL;
static triangle_lambertian *TRIANGLES_LAMBERTIAN;

static size_t NUM_OF_SPHERES_METAL;
static size_t NUM_OF_SPHERES_LAMBERTIAN;
static size_t NUM_OF_TRIANGLES_METAL;
static size_t NUM_OF_TRIANGLES_LAMBERTIAN;

sphere_metal *generate_sphere_ring(point center, double ring_radius, double sphere_radius, int count)
{
    sphere_metal *result =  malloc(sizeof(sphere_metal) * count);

    for (int i = 0; i < count; ++i)
    {
        double fuzz = (double)i / (double)count;
        color albedo = color_make(0.8, 0.6, 0.2);
        double angle = 2.0 * MY_PI * i / count;
        double x = center.x + ring_radius * cos(angle);
        double y = center.y + ring_radius * sin(angle);
        double z = center.z;
        result[i] = {
            .center = vec3_make(x, y, z),
            .radius = sphere_radius,
            .albedo = albedo,
            .fuzz = fuzz};
    }
    return result;
}

vec3 torus_point(point center, double R, double r, double u, double v)
{
    double x = (R + r * cos(v)) * cos(u);
    double y = r * sin(v);
    double z = (R + r * cos(v)) * sin(u);
    return vec3_add(center, vec3_make(x, y, z));
}

#endif