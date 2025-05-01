#ifndef WORLD_HITTABLE_H
#define WORLD_HITTABLE_H

// ここでは、 void* を使って vtable による統一的な書き方を目指す。

#include <stdlib.h>
#include <stdbool.h>
#include "vec3.h"
#include "component.h"

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

geometry create_sphere(sphere sph)
{
    sphere *sph_ptr = malloc(sizeof(sphere));
    *sph_ptr = sph;
    geometry g;
    g.hit_func = (hit_func_fn)hit_sphere;
    g.geometry = sph_ptr;
    return g;
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

material dielectric_material(dielectric d)
{
    dielectric *d_ptr = malloc(sizeof(dielectric));
    *d_ptr = d;
    material mat;
    mat.scatter = (scatter_fn)scatter_dielectric;
    mat.color_transform = (color_transform_fn)color_transform_dielectric;
    mat.data = d_ptr;

    return mat;
}

// ====== entity ======
typedef struct
{
    geometry geo;
    material mat;
} entity;

#endif