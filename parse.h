#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "vec3.h"
#include "component.h"

#define SCENE_FILENAME "scene.txt"

typedef struct
{
    geometry_type geo_type;
    sphere sph;
    triangle tri;
    material_type mat_type;
    lambertian lam;
    metal met;
    dielectric die;
} result;

static FILE *parse_fp = NULL;

size_t setup_file() {
    parse_fp = fopen(SCENE_FILENAME, "r");
    if (!parse_fp) {
        perror("fopen");
        exit(1);
    }

    char line[512];
    if (!fgets(line, sizeof(line), parse_fp)) {
        fprintf(stderr, "failed to read object count\n");
        exit(1);
    }

    size_t count = strtoul(line, NULL, 10);
    return count;
}

bool parse_line(result *res) {
    if (!parse_fp) return false;

    char line[512];
    if (!fgets(line, sizeof(line), parse_fp)) return false;

    char kind[32], shape_block[256];
    char material[32], mat_block[256];

    int matched = sscanf(line, "%31s { %255[^}] } %31s { %255[^}] }",
                         kind, shape_block, material, mat_block);

    if (matched != 4)
    {
        printf("parse error: %s\n", line);
        exit(1);
    }

    if (strcmp(kind, "sphere") == 0)
    {
        sphere s;
        sscanf(shape_block, "%lf %lf %lf %lf",
               &s.center.x, &s.center.y, &s.center.z, &s.radius);
        printf("sphere: center(%lf, %lf, %lf), radius(%lf)\n",
               s.center.x, s.center.y, s.center.z, s.radius);
        res->geo_type = SPHERE;
        res->sph = s;
    }
    else if (strcmp(kind, "triangle") == 0)
    {
        triangle t;
        sscanf(shape_block, "%lf %lf %lf %lf %lf %lf %lf %lf %lf",
               &t.a.x, &t.a.y, &t.a.z,
               &t.b.x, &t.b.y, &t.b.z,
               &t.c.x, &t.c.y, &t.c.z);
        printf("triangle: a(%lf, %lf, %lf), b(%lf, %lf, %lf), c(%lf, %lf, %lf)\n",
               t.a.x, t.a.y, t.a.z,
               t.b.x, t.b.y, t.b.z,
               t.c.x, t.c.y, t.c.z);
        res->geo_type = TRIANGLE;
        res->tri = t;
    }
    else
    {
        printf("unknown shape: %s\n", kind);
        exit(1);
    }

    if (strcmp(material, "lambertian") == 0)
    {
        lambertian l;
        sscanf(mat_block, "%lf %lf %lf",
               &l.albedo.x, &l.albedo.y, &l.albedo.z);
        printf("--lambertian: albedo(%lf, %lf, %lf)\n",
               l.albedo.x, l.albedo.y, l.albedo.z);
        res->mat_type = LAMBERTIAN;
        res->lam = l;
    }
    else if (strcmp(material, "metal") == 0)
    {
        metal m;
        sscanf(mat_block, "%lf %lf %lf %lf",
               &m.col.x, &m.col.y, &m.col.z, &m.fuzz);
        printf("--metal: col(%lf, %lf, %lf), fuzz(%lf)\n",
               m.col.x, m.col.y, m.col.z, m.fuzz);
        res->mat_type = METAL;
        res->met = m;
    }
    else if (strcmp(material, "dielectric") == 0)
    {
        dielectric d;
        sscanf(mat_block, "%lf %lf %lf %lf",
               &d.albedo.x, &d.albedo.y, &d.albedo.z, &d.ref_idx);
        printf("--dielectric: albedo(%lf, %lf, %lf)  ref(%lf)\n",
               d.albedo.x, d.albedo.y, d.albedo.z, d.ref_idx);
        res->mat_type = DIELECTRIC;
        res->die = d;
    }
    else
    {
        printf("unknown material: %s\n", material);
        exit(1);
    }

    return true;
}

#endif