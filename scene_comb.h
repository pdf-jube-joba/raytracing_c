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

#define VIEWPORT_HEIGHT 2.0
#define VIEWPORT_WIDTH 2.0
#define FOCAL_LENGTH 1.0

static vec3 CAMERA_ORIGIN;
static vec3 HORIZONTAL;
static vec3 VERTICAL;
static vec3 LOWER_LEFT_CORNER;

static sphere_metal *SPHERES_METAL = NULL;
static sphere_lambertian *SPHERES_LAMBERTIAN = NULL;
static triangle_metal *TRIANGLES_METAL = NULL;
static triangle_lambertian *TRIANGLES_LAMBERTIAN = NULL;

static size_t NUM_OF_SPHERES_METAL = 0;
static size_t NUM_OF_SPHERES_LAMBERTIAN = 0;
static size_t NUM_OF_TRIANGLES_METAL = 0;
static size_t NUM_OF_TRIANGLES_LAMBERTIAN = 0;

static size_t ENTITY_NUM;

sphere_metal *generate_sphere_metal_ring(point center, double ring_radius, double sphere_radius, int count)
{
    sphere_metal *result = malloc(sizeof(sphere_metal) * count);

    for (int i = 0; i < count; ++i)
    {
        double fuzz = 0.0;
        color albedo = color_make(0.9, 0.6, 0.8);
        double angle = 2.0 * MY_PI * i / count;
        double x = center.x + ring_radius * cos(angle);
        double y = center.y + ring_radius * sin(angle);
        double z = center.z;
        sphere sphere = {
            .center = vec3_make(x, y, z),
            .radius = sphere_radius};
        metal metal = {
            .col = albedo,
            .fuzz = fuzz};
        result[i] = (sphere_metal){
            .s = sphere,
            .m = metal};
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

triangle_lambertian *generate_torus_triangles(point center, double R, double r, int longs, int mers, size_t *out_count)
{
    int tri_count = longs * mers * 2;
    triangle_lambertian *result = malloc(sizeof(triangle_lambertian) * tri_count);
    *out_count = tri_count;

    for (int i = 0; i < longs; ++i)
    {
        for (int j = 0; j < mers; ++j)
        {
            double u0 = 2.0 * MY_PI * i / longs;
            double u1 = 2.0 * MY_PI * (i + 1) / longs;
            double v0 = 2.0 * MY_PI * j / mers;
            double v1 = 2.0 * MY_PI * (j + 1) / mers;

            // トーラスの点生成（XZ面回転）
            vec3 p00 = torus_point(center, R, r, u0, v0);
            vec3 p10 = torus_point(center, R, r, u1, v0);
            vec3 p01 = torus_point(center, R, r, u0, v1);
            vec3 p11 = torus_point(center, R, r, u1, v1);

            int index = 2 * (i * mers + j);

            // triangle 1
            triangle t1 = {
                .a = p00,
                .b = p10,
                .c = p11};
            // triangle 2
            triangle t2 = {
                .a = p00,
                .b = p11,
                .c = p01};
            // lambertian
            lambertian lambertian = {
                .albedo = color_make(0.8, 1.0, 0.8)};

            result[index + 0] = (triangle_lambertian){
                .t = t1,
                .m = lambertian};
            result[index + 1] = (triangle_lambertian){
                .t = t2,
                .m = lambertian};
        }
    }
    return result;
}

void setup_scene()
{
    CAMERA_ORIGIN = vec3_make(0.0, 0.0, 0.0);
    HORIZONTAL = vec3_make(VIEWPORT_WIDTH, 0.0, 0.0);
    VERTICAL = vec3_make(0.0, VIEWPORT_HEIGHT, 0.0);

    // (horizontal + vertical)/2 + (0, 0, focal_length)
    vec3 lower_left_corner_vector =
        vec3_add(vec3_scale(
                     vec3_add(HORIZONTAL, VERTICAL), 0.5),
                 vec3_make(0.0, 0.0, FOCAL_LENGTH));

    LOWER_LEFT_CORNER = vec3_sub(
        CAMERA_ORIGIN,
        lower_left_corner_vector);

    sphere_lambertian ground = {
        .s = {
            .center = vec3_make(0.0, -1000.0, 0.0),
            .radius = 999.0},
        .m = {.albedo = color_make(0.8, 0.8, 0.8)}};

    sphere_metal metal_sphere = {
        .s = {
            .center = vec3_make(1.5, 0.0, -5.0),
            .radius = 1.0},
        .m = {.col = color_make(0.8, 0.6, 0.2), .fuzz = 0.1}};

    sphere_lambertian lambertian_sphere = {
        .s = {
            .center = vec3_make(-1.5, 0.0, -5.0),
            .radius = 1.0},
        .m = {.albedo = color_make(0.3, 0.8, 0.2)}};

    SPHERES_LAMBERTIAN = malloc(sizeof(sphere_lambertian) * 2);
    SPHERES_LAMBERTIAN[0] = ground;
    SPHERES_LAMBERTIAN[1] = lambertian_sphere;
    NUM_OF_SPHERES_LAMBERTIAN = 2;

    SPHERES_METAL = malloc(sizeof(sphere_metal) * 1);
    SPHERES_METAL[0] = metal_sphere;
    NUM_OF_SPHERES_METAL = 1;

    // we do not use this
    // but copilot hallucinate with this
    TRIANGLES_LAMBERTIAN = malloc(sizeof(triangle_lambertian) * 1);
    NUM_OF_TRIANGLES_LAMBERTIAN = 0;
    // same here
    TRIANGLES_METAL = malloc(sizeof(triangle_metal) * 1);
    NUM_OF_TRIANGLES_METAL = 0;


    if (SPHERES_METAL == NULL || SPHERES_LAMBERTIAN == NULL ||
        TRIANGLES_METAL == NULL || TRIANGLES_LAMBERTIAN == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }

    // any static pointer of {SPHERES_METAL, SPHERES_LAMBERTIAN, TRIANGLES_METAL, TRIANGLES_LAMBERTIAN} is not NULL
    // so we can discuss without "there may null pointer"

    ENTITY_NUM = NUM_OF_SPHERES_METAL + NUM_OF_SPHERES_LAMBERTIAN + NUM_OF_TRIANGLES_METAL + NUM_OF_TRIANGLES_LAMBERTIAN;
}

#endif