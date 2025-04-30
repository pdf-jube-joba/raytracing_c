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

static entity* ENTITY;
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

    sphere ground = {
        .center = vec3_make(0.0, -1000.0, 0.0),
        .radius = 999.0};

    lambertian ground_material = {
        .albedo = color_make(0.8, 0.8, 0.8)};

    entity ground_entity = {
        .geo = create_sphere(ground),
        .mat = lambertian_material(ground_material)};

    sphere sphere1 = {
        .center = vec3_make(1.5, 0.0, -5.0),
        .radius = 1.0};

    metal metal_material1 = {
        .col = color_make(0.8, 0.6, 0.2),
        .fuzz = 0.1};

    entity metal_entity1 = {
        .geo = create_sphere(sphere1),
        .mat = metal_material(metal_material1)};

    sphere sphere2 = {
        .center = vec3_make(-1.5, 0.0, -5.0),
        .radius = 1.0};

    lambertian lambertian_material2 = {
        .albedo = color_make(0.3, 0.8, 0.2)};

    entity lambertian_entity2 = {
        .geo = create_sphere(sphere2),
        .mat = lambertian_material(lambertian_material2)};

    ENTITY = malloc(sizeof(entity) * 3);

    ENTITY[0] = ground_entity;
    ENTITY[1] = metal_entity1;
    ENTITY[2] = lambertian_entity2;
    ENTITY_NUM = 3;

}

#endif