#ifndef SCENE
#define SCENE

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "vec3.h"
#include "world_entity.h"
#include "parse.h"
#include "settings.h"

static vec3 CAMERA_ORIGIN;
static vec3 HORIZONTAL;
static vec3 VERTICAL;
static vec3 LOWER_LEFT_CORNER;

static entity *ENTITY;
static size_t ENTITY_NUM;

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

    size_t entity_count = setup_file();
    ENTITY = malloc(sizeof(entity) * entity_count);

    result res;
    while (parse_line(&res))
    {
        geometry geo;
        material mat;
        switch (res.geo_type)
        {
        case SPHERE:
            geo = create_sphere(res.sph);
            break;
        case TRIANGLE:
            geo = create_triangle(res.tri);
            break;
        }
        switch (res.mat_type)
        {
        case METAL:
            mat = metal_material(res.met);
            break;
        case LAMBERTIAN:
            mat = lambertian_material(res.lam);
            break;
        case DIELECTRIC:
            mat = dielectric_material(res.die);
            break;
        }
        ENTITY[ENTITY_NUM++] = (entity) { .geo = geo, .mat = mat };
    
    }
}

#endif