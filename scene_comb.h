#ifndef SCENE
#define SCENE

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "vec3.h"
#include "world_entity_comb.h"
#include "parse.h"

#define WIDTH 512
#define HEIGHT 512

#define VIEWPORT_HEIGHT 2.0
#define VIEWPORT_WIDTH 2.0
#define FOCAL_LENGTH 1.0

static vec3 CAMERA_ORIGIN;
static vec3 HORIZONTAL;
static vec3 VERTICAL;
static vec3 LOWER_LEFT_CORNER;

static entity *ENTITY = NULL;
static size_t ENTITY_NUM = 0;

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
        entity e;
        e.geo.type = res.geo_type;
        switch (res.geo_type)
        {
        case SPHERE:
            e.geo.geometry.s = res.sph;
            break;
        case TRIANGLE:
            e.geo.geometry.t = res.tri;
            break;
        default:
            break;
        }

        e.mat.type = res.mat_type;
        switch (res.mat_type)
        {
        case METAL:
            e.mat.material.m = res.met;
            break;
        case LAMBERTIAN:
            e.mat.material.l = res.lam;
            break;
        case DIELECTRIC:
            e.mat.material.d = res.die;
            break;
        default:
            break;
        }
        ENTITY[ENTITY_NUM++] = e;
    }
}

#endif