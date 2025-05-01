#ifndef COMPONENT_H
#define COMPONENT_H

// 共通して使う部分をまとめる

#include <stdlib.h>
#include <stdbool.h>
#include "vec3.h"

// ====== hit record ======

typedef struct
{
    ray r;
    double t;
    vec3 normal;
} hit_record_geometry;

// ray direction is from outside or not
bool hit_from_outer(hit_record_geometry rec)
{
    return vec3_dot(rec.normal, rec.r.direction) < 0.0;
}

point point_of_hit(hit_record_geometry rec)
{
    return ray_at(rec.r, rec.t);
}

bool hit_record_closer(hit_record_geometry *rec, hit_record_geometry new_rec)
{
    if (new_rec.t < 0.0)
    {
        return false;
    }
    if (rec->t < 0.0 || new_rec.t < rec->t)
    {
        *rec = new_rec;
        return true;
    }
    return false;
}

// ====== geometry ======

typedef enum
{
    SPHERE,
    TRIANGLE,
} geometry_type;

// ------ sphere ------

typedef struct
{
    point center;
    double radius;
} sphere;

hit_record_geometry hit_sphere(sphere *sph, ray ry)
{
    hit_record_geometry rec;
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

    return rec;
}

// ------ triangle ------
typedef struct
{
    point a, b, c;
} triangle;

hit_record_geometry hit_triangle(triangle *tri, ray ry)
{
    hit_record_geometry rec;
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

    return rec;
}

// ====== materials ======

typedef enum
{
    METAL,
    LAMBERTIAN,
    DIELECTRIC,
} material_type;

// ------ metal ------

typedef struct
{
    color col;
    double fuzz;
} metal;

ray scatter_metal(metal *m, hit_record_geometry rec, unsigned int *state)
{
    vec3 reflected = vec3_reflect(vec3_unit(rec.r.direction), rec.normal);
    ray r = ray_make(point_of_hit(rec), reflected);
    vec3 fuzz_vec = vec3_scale(random_unit_vector(state), m->fuzz);
    r.direction = vec3_add(r.direction, fuzz_vec);
    return r;
}

color color_transform_metal(metal *m, color col, unsigned int *state)
{
    color albedo = m->col;
    return color_attenuation(col, albedo);
}

// ------ lambertian ------

typedef struct
{
    color albedo;
} lambertian;

ray scatter_lambertian(lambertian *l, hit_record_geometry rec, unsigned int *state)
{
    vec3 target = vec3_add(rec.normal, random_unit_vector(state));
    if (vec3_length(target) < 0.001)
    {
        target = rec.normal;
    }
    ray r = ray_make(point_of_hit(rec), target);
    return r;
}

color color_transform_lambertian(lambertian *l, color col, unsigned int *state)
{
    color albedo = l->albedo;
    return color_attenuation(col, albedo);
}

// ------ dielectric ------
typedef struct
{
    color albedo;
    double ref_idx;
} dielectric;

ray scatter_dielectric(dielectric *d, hit_record_geometry rec, unsigned int *state)
{
    vec3 outward_normal;
    bool is_front = hit_from_outer(rec);
    outward_normal = is_front ? rec.normal : vec3_inv(rec.normal);

    double etai_over_etat = is_front ? (1.0 / d->ref_idx): d->ref_idx;

    vec3 unit_direction = vec3_unit(rec.r.direction);

    double cos_theta = fmin(vec3_dot(vec3_inv(unit_direction), outward_normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    if (etai_over_etat * sin_theta > 1.0)
    {
        // total internal reflection
        vec3 reflected = vec3_reflect(unit_direction, outward_normal);
        return ray_make(point_of_hit(rec), reflected);
    }

    double reflect_prob = schlick(cos_theta, etai_over_etat);

    if (rand_unit(state) < reflect_prob)
    {
        vec3 reflected = vec3_reflect(unit_direction, outward_normal);
        return ray_make(point_of_hit(rec), reflected);
    }
    vec3 refracted = refract(unit_direction, outward_normal, etai_over_etat);
    return ray_make(point_of_hit(rec), refracted);
}

color color_transform_dielectric(dielectric *d, color col, unsigned int *state)
{
    color albedo = d->albedo;
    return color_attenuation(col, albedo);
}

#endif