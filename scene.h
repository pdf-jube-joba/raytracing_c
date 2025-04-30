#ifndef SCENE
#define SCENE

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "vec3.h"
#include "world_entity.h"

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

static sphere *SPHERES;
static size_t NUM_OF_SPHERES;
static triangle *TRIANGLES;
static size_t NUM_OF_TRIANGLES;

sphere *generate_sphere_ring(point center, double ring_radius, double sphere_radius, int count)
{
    sphere *result = malloc(sizeof(sphere) * count);

    for (int i = 0; i < count; ++i)
    {
        double fuzz = (double)i / (double)count;
        material *mat = metal_material(color_make(0.8, 0.6, 0.2), fuzz);
        double angle = 2.0 * MY_PI * i / count;
        double x = center.x + ring_radius * cos(angle);
        double y = center.y + ring_radius * sin(angle);
        double z = center.z;
        result[i] = sphere_make(vec3_make(x, y, z), sphere_radius, mat);
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

triangle *generate_torus_triangles(point center, double R, double r, int longs, int mers, size_t *out_count)
{
    int tri_count = longs * mers * 2;
    triangle *result = malloc(sizeof(triangle) * tri_count);
    *out_count = tri_count;

    material *mat = lambertian_material(color_make(1.0, 1.0, 1.0));

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
            result[index + 0] = triangle_make(p00, p10, p11, mat);
            result[index + 1] = triangle_make(p00, p11, p01, mat);
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

    int num_of_spheres = 8;

    SPHERES = generate_sphere_ring(vec3_make(0.0, 0.0, -10.0), 5.5, 1.5, num_of_spheres);
    NUM_OF_SPHERES = num_of_spheres;

    TRIANGLES = generate_torus_triangles(vec3_make(0.0, -2.0, -10.0), 3.5, 0.5, 16, 8, &NUM_OF_TRIANGLES);
}

#endif