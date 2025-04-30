#include <stdio.h>
#include <sys/time.h> // for time
#include <limits.h>   // UINT_MAX

#include "vec3.h"
#include "world_entity_comb.h"
#include "scene_comb.h"

#define SAMPLING 128
#define MAX_REFLECTION_DEPTH 5

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec - st.tv_sec) + (et.tv_usec - st.tv_usec) / 1000000.0;
}

// if there is a hit
// - subst hit_record_geometry
// - subst material_type
// - subst either metal or lambertian
void world_hit(ray r, hit_record_geometry *closest, material_type *mt, metal *material_metal, lambertian *material_lambertian)
{
    *closest = (hit_record_geometry){.t = -1.0};

    for (size_t i = 0; i < NUM_OF_SPHERES_METAL; ++i)
    {
        hit_record_geometry rec = hit_sphere(&SPHERES_METAL[i].s, r);
        if (hit_record_closer(closest, rec))
        {
            *mt = METAL;
            *material_metal = SPHERES_METAL[i].m;
        }
    }

    for (size_t i = 0; i < NUM_OF_SPHERES_LAMBERTIAN; ++i)
    {
        hit_record_geometry rec = hit_sphere(&SPHERES_LAMBERTIAN[i].s, r);
        if (hit_record_closer(closest, rec))
        {
            *mt = LAMBERTIAN;
            *material_lambertian = SPHERES_LAMBERTIAN[i].m;
        }
    }

    for (size_t i = 0; i < NUM_OF_TRIANGLES_METAL; ++i)
    {
        hit_record_geometry rec = hit_triangle(&TRIANGLES_METAL[i].t, r);
        if (hit_record_closer(closest, rec))
        {
            *mt = METAL;
            *material_metal = TRIANGLES_METAL[i].m;
        }
    }

    for (size_t i = 0; i < NUM_OF_TRIANGLES_LAMBERTIAN; ++i)
    {
        hit_record_geometry rec = hit_triangle(&TRIANGLES_LAMBERTIAN[i].t, r);
        if (hit_record_closer(closest, rec))
        {
            *mt = LAMBERTIAN;
            *material_lambertian = TRIANGLES_LAMBERTIAN[i].m;
        }
    }
}

color ray_color(ray r, unsigned int *state)
{

    material_type mts[MAX_REFLECTION_DEPTH];

    metal hit_metals[MAX_REFLECTION_DEPTH];
    int hit_metals_count = 0;
    lambertian hit_lambertian[MAX_REFLECTION_DEPTH];
    int hit_lambertian_count = 0;

    int reflection_depth = 0;

    for (reflection_depth = 0; reflection_depth < MAX_REFLECTION_DEPTH; ++reflection_depth)
    {
        hit_record_geometry closest;
        material_type m;
        world_hit(r, &closest, &m, &hit_metals[hit_metals_count], &hit_lambertian[hit_lambertian_count]);

        if (closest.t < 0.0)
        {
            // no hit
            break;
        }
        else
        {
            // hit
            switch (m)
            {
            case METAL:
                r = scatter_metal(&hit_metals[hit_metals_count], closest, state);
                mts[reflection_depth] = METAL;
                hit_metals_count++;
                break;
            case LAMBERTIAN:
                r = scatter_lambertian(&hit_lambertian[hit_lambertian_count], closest, state);
                mts[reflection_depth] = LAMBERTIAN;
                hit_lambertian_count++;
                break;
            default:
                break;
            }
        }
    }

    // first color
    color pixel_color;
    if (reflection_depth >= MAX_REFLECTION_DEPTH)
    {
        // printf("find! %f %f %f\n", r.direction.x, r.direction.y, r.direction.z);
        pixel_color = color_make(0.8, 0.8, 0.8);
    }
    else
    {
        vec3 unit_direction = vec3_unit(r.direction);
        double t = 0.5 * (unit_direction.y + 1.0);
        pixel_color = vec3_add(
            vec3_scale(vec3_make(1.0, 1.0, 1.0), 1.0 - t),
            vec3_scale(vec3_make(0.5, 0.7, 1.0), t));
    }

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
    unsigned int global_seed = rand();

    double total_time = 0.0;

    #pragma omp parallel for
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
        printf("%i %f sec\n", y, total_time);
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
    render(image);
    save_ppm("ri_comb_omp.ppm", image);
    return 0;
}
