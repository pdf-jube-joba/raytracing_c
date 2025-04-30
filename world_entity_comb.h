#ifndef WORLD_HITTABLE_H
#define WORLD_HITTABLE_H

// ここでは、 geometry と material を combinatorial に定義する方式をとってみる。

#include <stdlib.h>
#include <stdbool.h>
#include "vec3.h"

// ====== hit record ======

// enum of entity
typedef enum
{
    SPHERE_METAL,
    SPHERE_LAMBERTIAN,
    TRIANGLE_METAL,
    TRIANGLE_LAMBERTIAN
} entity_type;

// material forward declaration
typedef struct
{
    ray r;        // ヒットした光
    double t;     // ヒットした時間
    vec3 normal;  // ヒットした面の normal
    entity_type en; // entity の種類
    void *entity; // entity へのポインタ

} hit_record;

void hit_record_closer(hit_record *a, hit_record b)
{
    if (b.t < 0.0)
        return;
    if (a->t < 0.0 || a->t > b.t)
    {
        a->t = b.t;
        a->r = b.r;
        a->normal = b.normal;
        a->en = b.en;
        a->entity = b.entity;
    }
}

// ray direction is from outside or not
bool hit_from_outer(hit_record rec)
{
    return vec3_dot(rec.normal, vec3_inv(rec.r.direction)) > 0.0;
}

point point_of_hit(hit_record rec)
{
    return ray_at(rec.r, rec.t);
}

// ====== spheres ======

typedef struct
{
    point center;
    double radius;
    color albedo;
    double fuzz;
} sphere_metal;

hit_record hit_sphere_metal(sphere_metal *sph, ray ry)
{
    hit_record rec;
    rec.t = -1.0;

    vec3 oc = vec3_sub(ry.origin, sph->center);
    double a = vec3_dot(ry.direction, ry.direction);
    double b = 2.0 * vec3_dot(oc, ry.direction);
    double c = vec3_dot(oc, oc) - sph->radius * sph->radius;
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return rec; // No hit

    double t = (-b - sqrt(discriminant)) / (2.0 * a);
    if (t < 0.001)
        return rec;

    rec.t = t;
    rec.r = ry;
    vec3 outward_normal = vec3_unit(vec3_sub(ray_at(ry, t), sph->center));
    rec.normal = outward_normal;
    rec.en = SPHERE_METAL;
    rec.entity = sph;

    return rec;
}

ray scatter_sphere_metal(sphere_metal *sph, hit_record rec, unsigned int *state)
{
    vec3 reflected = vec3_reflect(vec3_unit(rec.r.direction), rec.normal);
    ray r = ray_make(point_of_hit(rec), reflected);
    vec3 fuzz_vec = vec3_scale(random_unit_vector(state), sph->fuzz);
    r.direction = vec3_add(r.direction, fuzz_vec);
    return r;
}

color color_transform_sphere_metal(sphere_metal *sph, hit_record rec, color col, unsigned int *state)
{
    color albedo = sph->albedo;
    return color_attenuation(col, albedo);
}

typedef struct
{
    point center;
    double radius;
    color albedo;
} sphere_lambertian;

hit_record hit_sphere_lambertian(sphere_lambertian *sph, ray ry)
{
    hit_record rec;
    rec.t = -1.0;

    vec3 oc = vec3_sub(ry.origin, sph->center);
    double a = vec3_dot(ry.direction, ry.direction);
    double b = 2.0 * vec3_dot(oc, ry.direction);
    double c = vec3_dot(oc, oc) - sph->radius * sph->radius;
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return rec; // No hit

    double t = (-b - sqrt(discriminant)) / (2.0 * a);
    if (t < 0.001)
        return rec;

    rec.t = t;
    rec.r = ry;
    vec3 outward_normal = vec3_unit(vec3_sub(ray_at(ry, t), sph->center));
    rec.normal = outward_normal;
    rec.en = SPHERE_LAMBERTIAN;
    rec.entity = sph;

    return rec;
}

ray scatter_sphere_lambertian(sphere_lambertian *sph, hit_record rec, unsigned int *state)
{
    vec3 target = vec3_add(rec.normal, random_unit_vector(state));
    ray r = ray_make(point_of_hit(rec), target);
    return r;
}

color color_transform_sphere_lambertian(sphere_lambertian *sph, hit_record rec, color col, unsigned int *state)
{
    color albedo = sph->albedo;
    return color_attenuation(col, albedo);
}

// ====== triangles ======

typedef struct
{
    point a, b, c;
    color albedo;
    double fuzz;
} triangle_metal;

hit_record hit_triangle_metal(triangle_metal *tri, ray ry)
{
    hit_record rec;
    rec.t = -1.0;

    vec3 ab = vec3_sub(tri->b, tri->a);
    vec3 ac = vec3_sub(tri->c, tri->a);
    vec3 pvec = vec3_cross(ry.direction, ac);
    double det = vec3_dot(ab, pvec);

    if (det < 0.001)
        return rec;

    double inv_det = 1.0 / det;

    vec3 tvec = vec3_sub(ry.origin, tri->a);
    double u = inv_det * vec3_dot(tvec, pvec);

    if (u < 0.0 || u > 1.0)
        return rec;

    vec3 qvec = vec3_cross(tvec, ab);
    double v = inv_det * vec3_dot(ry.direction, qvec);

    if (v < 0.0 || u + v > 1.0)
        return rec;

    double t = inv_det * vec3_dot(ac, qvec);

    if (t < 0.001)
        return rec;

    rec.t = t;
    rec.r = ry;
    vec3 outward_normal = vec3_unit(vec3_cross(ab, ac));
    rec.normal = outward_normal;
    rec.en = TRIANGLE_METAL;
    rec.entity = tri;

    return rec;
}

ray scatter_triangle_metal(triangle_metal *tri, hit_record rec, unsigned int *state)
{
    vec3 reflected = vec3_reflect(vec3_unit(rec.r.direction), rec.normal);
    ray r = ray_make(point_of_hit(rec), reflected);
    vec3 fuzz_vec = vec3_scale(random_unit_vector(state), tri->fuzz);
    r.direction = vec3_add(r.direction, fuzz_vec);
    return r;
}

color color_transform_triangle_metal(triangle_metal *tri, hit_record rec, color col, unsigned int *state)
{
    color albedo = tri->albedo;
    return color_attenuation(col, albedo);
}

typedef struct
{
    point a, b, c;
    color albedo;
} triangle_lambertian;

hit_record hit_triangle_lambertian(triangle_lambertian *tri, ray ry)
{
    hit_record rec;
    rec.t = -1.0;

    vec3 ab = vec3_sub(tri->b, tri->a);
    vec3 ac = vec3_sub(tri->c, tri->a);
    vec3 pvec = vec3_cross(ry.direction, ac);
    double det = vec3_dot(ab, pvec);

    if (det < 0.001)
        return rec;

    double inv_det = 1.0 / det;

    vec3 tvec = vec3_sub(ry.origin, tri->a);
    double u = inv_det * vec3_dot(tvec, pvec);

    if (u < 0.0 || u > 1.0)
        return rec;

    vec3 qvec = vec3_cross(tvec, ab);
    double v = inv_det * vec3_dot(ry.direction, qvec);

    if (v < 0.0 || u + v > 1.0)
        return rec;

    double t = inv_det * vec3_dot(ac, qvec);

    if (t < 0.001)
        return rec;

    rec.t = t;
    rec.r = ry;
    vec3 outward_normal = vec3_unit(vec3_cross(ab, ac));
    rec.normal = outward_normal;
    rec.en = TRIANGLE_LAMBERTIAN;
    rec.entity = tri;

    return rec;
}

ray scatter_triangle_lambertian(triangle_lambertian *tri, hit_record rec, unsigned int *state)
{
    vec3 target = vec3_add(rec.normal, random_unit_vector(state));
    ray r = ray_make(point_of_hit(rec), target);
    return r;
}

color color_transform_triangle_lambertian(triangle_lambertian *tri, hit_record rec, color col, unsigned int *state)
{
    color albedo = tri->albedo;
    return color_attenuation(col, albedo);
}

#endif