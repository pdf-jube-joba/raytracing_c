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

// ====== entity ======

typedef struct
{
    geometry_union geo;
    material_union mat;
} entity;

#endif