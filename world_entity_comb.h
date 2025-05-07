#ifndef WORLD_HITTABLE_H
#define WORLD_HITTABLE_H

// ここでは、 geometry と material を union による列挙型を用いてまとめる。

#include <stdlib.h>
#include <stdbool.h>
#include "vec3.h"
#include "component.h"

// ====== geometry union ======

typedef struct
{
    geometry_type type;
    union
    {
        sphere s;
        triangle t;
    } geometry;
} geometry_union;

// ====== material union ======

typedef struct
{
    material_type type;
    union
    {
        metal m;
        lambertian l;
        dielectric d;
    } material;
} material_union;

hit_record_geometry hit_geometry(geometry_union g, ray ry)
{
    hit_record_geometry rec;
    switch (g.type)
    {
    case SPHERE:
        rec = hit_sphere(&g.geometry.s, ry);
        break;
    case TRIANGLE:
        rec = hit_triangle(&g.geometry.t, ry);
        break;
    default:
        rec.t = -1.0;
        break;
    }
    return rec;
}

// ====== entity ======

typedef struct
{
    geometry_union geo;
    material_union mat;
} entity;

ray scatter_material(material_union mu, hit_record_geometry rec, unsigned int *state)
{
    switch (mu.type)
    {
    case METAL:
        return scatter_metal(&mu.material.m, rec, state);
    case LAMBERTIAN:
        return scatter_lambertian(&mu.material.l, rec, state);
    case DIELECTRIC:
        return scatter_dielectric(&mu.material.d, rec, state);
    default:
        return (ray){0};
    }
}

color color_transform_material(material_union mu, color col, unsigned int *state)
{
    switch (mu.type)
    {
    case METAL:
        return color_transform_metal(&mu.material.m, col, state);
    case LAMBERTIAN:
        return color_transform_lambertian(&mu.material.l, col, state);
    case DIELECTRIC:
        return color_transform_dielectric(&mu.material.d, col, state);
    default:
        return (color){0};
    }
}

#endif