#ifndef WORLD_HITTABLE_H
#define WORLD_HITTABLE_H

#include <stdlib.h>
#include <stdbool.h>
#include "vec3.h"

// ====== hit record ======

// material forward declaration
typedef struct material material;

typedef struct
{
    ray r;         // ヒットした光
    double t;      // ヒットした時間
    vec3 normal;   // ヒットした面の normal
    material *mat; // マテリアル
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
        a->mat = b.mat;
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

// ====== geometry ======

// **** sphere ****

typedef struct
{
    point center;
    double radius;
    material *mat;
} sphere;

const sphere sphere_make(point center, double radius, material *mat)
{
    assert(radius > 0.0);
    sphere s;
    s.center = center;
    s.radius = radius;
    s.mat = mat;
    return s;
}

hit_record hit_sphere(sphere sph, ray ry)
{
    hit_record rec;
    rec.t = -1.0;

    vec3 oc = vec3_sub(ry.origin, sph.center);
    double a = vec3_dot(ry.direction, ry.direction);
    double b = 2.0 * vec3_dot(oc, ry.direction);
    double c = vec3_dot(oc, oc) - sph.radius * sph.radius;
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return rec; // No hit

    double t = (-b - sqrt(discriminant)) / (2.0 * a);
    if (t < 0.001)
        return rec;

    rec.t = t;
    rec.r = ry;
    vec3 outward_normal = vec3_unit(vec3_sub(ray_at(ry, t), sph.center));
    rec.normal = outward_normal;
    rec.mat = sph.mat;

    return rec;
}

// **** triangle ****

typedef struct
{
    point a, b, c;
    material *mat;
} triangle;

triangle triangle_make(point a, point b, point c, material *mat)
{
    triangle t;
    t.a = a;
    t.b = b;
    t.c = c;
    t.mat = mat;
    return t;
}

hit_record hit_triangle(triangle tri, ray ry)
{
    hit_record rec;
    rec.t = -1.0;

    vec3 ab = vec3_sub(tri.b, tri.a);
    vec3 ac = vec3_sub(tri.c, tri.a);
    vec3 pvec = vec3_cross(ry.direction, ac);
    double det = vec3_dot(ab, pvec);

    if (det < 1e-8 && det > -1e-8)
        return rec;

    double inv_det = 1.0 / det;

    vec3 tvec = vec3_sub(ry.origin, tri.a);
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
    rec.normal = vec3_inv(outward_normal);
    rec.mat = tri.mat;

    return rec;
}

// ====== material ======

typedef ray (*scatter_fn)(const void *self, hit_record rec, unsigned int *state);
typedef color (*color_transform_fn)(const void *self, hit_record rec, color col, unsigned int *state);

typedef struct material
{
    scatter_fn scatter;
    color_transform_fn color_transform;
    void *data; // 追加データ
} material;

// **** lambertian ****

typedef struct
{
    color albedo;
} lambertian_data;

ray lambertian_scatter(const void *self, hit_record rec, unsigned int *state)
{
    vec3 target = vec3_add(rec.normal, random_unit_vector(state));
    ray r = ray_make(point_of_hit(rec), target);
    return r;
}

color lambertian_color_transform(const void *self, hit_record rec, color col, unsigned int *state)
{
    lambertian_data *data = (lambertian_data *)self;
    color albedo = data->albedo;
    return color_attenuation(col, albedo);
}

material *lambertian_material(color albedo)
{
    lambertian_data *data = malloc(sizeof(lambertian_data));
    data->albedo = albedo;

    material *lambertian_material = malloc(sizeof(material));
    lambertian_material->scatter = lambertian_scatter;
    lambertian_material->color_transform = lambertian_color_transform;
    lambertian_material->data = data;
    return lambertian_material;
}

// **** metal ***

typedef struct
{
    color albedo;
    double fuzz;
} metal_data;

ray metal_scatter(const void *self, hit_record rec, unsigned int *state)
{
    metal_data *data = (metal_data *)self;
    vec3 reflected = vec3_reflect(vec3_unit(rec.r.direction), rec.normal);
    ray r = ray_make(point_of_hit(rec), reflected);
    vec3 fuzz_vec = vec3_scale(random_unit_vector(state), data->fuzz);
    r.direction = vec3_add(r.direction, fuzz_vec);
    return r;
}

color metal_color_transform(const void *self, hit_record rec, color col, unsigned int *state)
{
    metal_data *data = (metal_data *)self;
    color albedo = data->albedo;
    return color_attenuation(col, albedo);
}

material *metal_material(color albedo, double fuzz)
{
    metal_data *data = malloc(sizeof(metal_data));
    data->albedo = albedo;
    data->fuzz = fuzz;

    material *metal_material = malloc(sizeof(material));
    metal_material->scatter = metal_scatter;
    metal_material->color_transform = metal_color_transform;
    metal_material->data = data;
    return metal_material;
}

#endif