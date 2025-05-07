#include <stdio.h>
#include <sys/time.h> // for time

#include "vec3.h"
#include "world_entity_comb.h"
#include "scene_comb.h"

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec - st.tv_sec) + (et.tv_usec - st.tv_usec) / 1000000.0;
}

material_union world_hit(ray r, hit_record_geometry *closest)
{
    material_union mu;
    *closest = (hit_record_geometry){.t = -1.0};

    for (size_t i = 0; i < ENTITY_NUM; ++i)
    {
        entity e = ENTITY[i];
        bool b = false;
        ;
        switch (e.geo.type)
        {
        case SPHERE:
            b = hit_record_closer(closest, hit_sphere(&e.geo.geometry.s, r));
            break;
        case TRIANGLE:
            b = hit_record_closer(closest, hit_triangle(&e.geo.geometry.t, r));
            break;
        default:
            break;
        }

        if (b)
        {
            mu.type = e.mat.type;
            switch (e.mat.type)
            {
            case METAL:
                mu.material.m = e.mat.material.m;
                break;
            case LAMBERTIAN:
                mu.material.l = e.mat.material.l;
                break;
            case DIELECTRIC:
                mu.material.d = e.mat.material.d;
                break;
            default:
                break;
            }
        }
    }
    return mu;
}

color ray_color(ray r, unsigned int *state)
{
    material_type mts[MAX_REFLECTION_DEPTH];

    metal hit_metals[MAX_REFLECTION_DEPTH];
    int hit_metals_count = 0;
    lambertian hit_lambertian[MAX_REFLECTION_DEPTH];
    int hit_lambertian_count = 0;
    dielectric hit_dielectric[MAX_REFLECTION_DEPTH];
    int hit_dielectric_count = 0;

    int reflection_depth = 0;

    for (reflection_depth = 0; reflection_depth < MAX_REFLECTION_DEPTH; ++reflection_depth)
    {
        hit_record_geometry closest;
        material_union mu;
        mu = world_hit(r, &closest);

        if (closest.t < 0.0)
        {
            // no hit
            break;
        }
        else
        {
            // hit
            switch (mu.type)
            {
            case METAL:
                hit_metals[hit_metals_count] = mu.material.m;
                r = scatter_metal(&hit_metals[hit_metals_count], closest, state);
                mts[reflection_depth] = METAL;
                hit_metals_count++;
                break;
            case LAMBERTIAN:
                hit_lambertian[hit_lambertian_count] = mu.material.l;
                r = scatter_lambertian(&hit_lambertian[hit_lambertian_count], closest, state);
                mts[reflection_depth] = LAMBERTIAN;
                hit_lambertian_count++;
                break;
            case DIELECTRIC:
                hit_dielectric[hit_dielectric_count] = mu.material.d;
                r = scatter_dielectric(&hit_dielectric[hit_dielectric_count], closest, state);
                mts[reflection_depth] = DIELECTRIC;
                hit_dielectric_count++;
                break;
            default:
                break;
            }
        }
    }

    color pixel_color = background_color(r);

    // compute color by reverse order
    for (int i = reflection_depth - 1; i >= 0; --i)
    {
        switch (mts[i])
        {
        case METAL:
            hit_metals_count--;
            pixel_color = color_transform_metal(&hit_metals[hit_metals_count], pixel_color, state);
            break;
        case LAMBERTIAN:
            hit_lambertian_count--;
            pixel_color = color_transform_lambertian(&hit_lambertian[hit_lambertian_count], pixel_color, state);
            break;
        case DIELECTRIC:
            hit_dielectric_count--;
            pixel_color = color_transform_dielectric(&hit_dielectric[hit_dielectric_count], pixel_color, state);
            break;
        default:
            break;
        }
    }

    return pixel_color;
}

void render(color image[HEIGHT][WIDTH])
{
    struct timeval t1, t2;

    // seed for random number generation
    unsigned int global_seed = RANDOM_SEED_GLOBAL;

    double total_time = 0.0;

    for (int y = HEIGHT - 1; y >= 0; --y)
    {

        gettimeofday(&t1, NULL);
        for (int x = 0; x < WIDTH; ++x)
        {

            color col = color_make(0.0, 0.0, 0.0);

            for (int s = 0; s < SAMPLING; ++s)
            {
                // random number in [0, 1)
                double x_offset = rand_unit(&global_seed);
                double y_offset = rand_unit(&global_seed);
                double u = ((double)x + x_offset) / (WIDTH - 1);
                double v = ((double)y + y_offset) / (HEIGHT - 1);

                vec3 direction = vec3_add(
                    LOWER_LEFT_CORNER,
                    vec3_add(
                        vec3_scale(HORIZONTAL, u),
                        vec3_scale(VERTICAL, v)));

                ray r = ray_make(CAMERA_ORIGIN, direction);
                col = vec3_add(col, ray_color(r, &global_seed));
            }

            col = vec3_scale(col, 1.0 / SAMPLING);
            image[HEIGHT - 1 - y][x] = col;
        }

        gettimeofday(&t2, NULL);
        total_time = time_diff_sec(t1, t2);
        // printf("%i %f sec\n", y, total_time);
    }
}

void save_ppm(const char *filename, color image[HEIGHT][WIDTH])
{
    FILE *f = fopen(filename, "wb");
    if (!f)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    fprintf(f, "P3\n%d %d\n255\n", WIDTH, HEIGHT);
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
        {
            write_color(f, image[y][x]);
        }
    fclose(f);
}

int main(int argc, char *argv[])
{
    setup_scene();
    color image[HEIGHT][WIDTH];

    struct timeval t1, t2;
    gettimeofday(&t1, NULL);

    render(image);

    gettimeofday(&t2, NULL);
    double total_time = time_diff_sec(t1, t2);
    printf("render done %f sec\n", total_time);

    save_ppm("ri_comb.ppm", image);
    return 0;
}
