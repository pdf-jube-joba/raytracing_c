#ifndef WORLD_HITTABLE_H
#define WORLD_HITTABLE_H

// ここでは、 void* を使って vtable による統一的な書き方を目指す。

#include <stdlib.h>
#include <stdbool.h>
#include "vec3.h"

// ====== hit record ======

typedef struct
{
    ray r;       // ヒットした光
    double t;    // ヒットした時間
    vec3 normal; // ヒットした面の normal
} hit_record_geometry;

// ray direction is from outside or not
bool hit_from_outer(hit_record_geometry rec)
{
    return vec3_dot(rec.normal, vec3_inv(rec.r.direction)) > 0.0;
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

typedef hit_record_geometry (*hit_func_fn)(void *geometry, ray ry);

typedef struct
{
    hit_func_fn hit_func;
    void *geometry;
} geometry;

hit_record_geometry hit_geometry(geometry g, ray ry)
{
    return g.hit_func(g.geometry, ry);
}

// ------ sphere ------

typedef struct
{
    point center;
    double radius;
} sphere;

hit_record_geometry hit_sphere(void *geometry, ray ry)
{
    sphere *sph = (sphere *)geometry;
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

geometry create_sphere(sphere sph)
{
    sphere *sph_ptr = malloc(sizeof(sphere));
    *sph_ptr = sph;
    geometry g;
    g.hit_func = (hit_func_fn)hit_sphere;
    g.geometry = sph_ptr;
    return g;
}

// ------ triangle ------
typedef struct
{
    point a, b, c;
} triangle;

hit_record_geometry hit_triangle(void *geometry, ray ry)
{
    triangle *tri = (triangle *)geometry;
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

geometry create_triangle(triangle tri)
{
    triangle *tri_ptr = malloc(sizeof(triangle));
    *tri_ptr = tri;
    geometry g;
    g.hit_func = (hit_func_fn)hit_triangle;
    g.geometry = tri_ptr;
    return g;
}

// ====== materials ======

typedef ray (*scatter_fn)(void *material, hit_record_geometry rec, unsigned int *state);
typedef color (*color_transform_fn)(void *material, color col, unsigned int *state);

typedef struct
{
    scatter_fn scatter;
    color_transform_fn color_transform;
    void *data;
} material;

ray scatter_material(material mat, hit_record_geometry rec, unsigned int *state)
{
    return mat.scatter(mat.data, rec, state);
}

color color_transform_material(material mat, color col, unsigned int *state)
{
    return mat.color_transform(mat.data, col, state);
}

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

material metal_material(metal m)
{
    metal *m_ptr = malloc(sizeof(metal));
    *m_ptr = m;

    material mat;
    mat.scatter = (scatter_fn)scatter_metal;
    mat.color_transform = (color_transform_fn)color_transform_metal;
    mat.data = m_ptr;

    return mat;
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

material lambertian_material(lambertian l)
{
    lambertian *l_ptr = malloc(sizeof(lambertian));
    *l_ptr = l;
    material mat;
    mat.scatter = (scatter_fn)scatter_lambertian;
    mat.color_transform = (color_transform_fn)color_transform_lambertian;
    mat.data = l_ptr;

    return mat;
}

// ====== entity ======
typedef struct
{
    geometry geo;
    material mat;
} entity;

#endif