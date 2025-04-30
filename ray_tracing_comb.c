#include <stdio.h>
#include <sys/time.h> // for time
#include <limits.h>   // UINT_MAX

#include "vec3.h"
#include "world_entity_comb.h"
#include "scene_comb.h"

#define SAMPLING 16
#define MAX_REFLECTION_DEPTH 5

double time_diff_sec(struct timeval st, struct timeval et)
{
    return (double)(et.tv_sec - st.tv_sec) + (et.tv_usec - st.tv_usec) / 1000000.0;
}

hit_record world_hit(ray r)
{
    hit_record closest = {.t = -1.0};

    for (size_t i = 0; i < NUM_OF_SPHERES_METAL; ++i)
    {
        hit_record rec = hit_sphere_metal(&SPHERES_METAL[i], r);
        hit_record_closer(&closest, rec);
    }

    for (size_t i = 0; i < NUM_OF_SPHERES_LAMBERTIAN; ++i)
    {
        hit_record rec = hit_sphere_metal(&SPHERES_LAMBERTIAN[i], r);
        hit_record_closer(&closest, rec);
    }

    for (size_t i = 0; i < NUM_OF_TRIANGLES_METAL; ++i)
    {
        hit_record rec = hit_triangle_metal(&TRIANGLES_METAL[i], r);
        hit_record_closer(&closest, rec);
    }

    for (size_t i = 0; i < NUM_OF_TRIANGLES_LAMBERTIAN; ++i)
    {
        hit_record rec = hit_triangle_lambertian(&TRIANGLES_LAMBERTIAN[i], r);
        hit_record_closer(&closest, rec);
    }

    return closest;
}

color ray_color(ray r, int *state)
{
    hit_record hit_records[MAX_REFLECTION_DEPTH];

    int reflection_depth = 0;

    for (reflection_depth = 0; reflection_depth < MAX_REFLECTION_DEPTH; ++reflection_depth)
    {
        hit_record closest = world_hit(r);

        if (closest.t < 0.0)
        {
            // no hit
            break;
        }
        else
        {
            // hit

            // new ray
            switch (closest.en)
            {
            case SPHERE_METAL:
                r = scatter_sphere_metal(closest.entity, closest, state);
                break;
            case SPHERE_LAMBERTIAN:
                r = scatter_sphere_lambertian(closest.entity, closest, state);
                break;
            case TRIANGLE_METAL:
                r = scatter_triangle_metal(closest.entity, closest, state);
                break;
            case TRIANGLE_LAMBERTIAN:
                r = scatter_triangle_lambertian(closest.entity, closest, state);
                break;
            default:
                break;
            }
            hit_records[reflection_depth] = closest;
        }
    }

    // first color
    color pixel_color = color_make(1.0, 1.0, 1.0);
    if (reflection_depth >= MAX_REFLECTION_DEPTH)
    {
        // printf("find! %f %f %f\n", r.direction.x, r.direction.y, r.direction.z);
        pixel_color = color_make(1.0, 1.0, 1.0);
    }
    else
    {
        vec3 unit_direction = vec3_unit(r.direction);
        double t = 0.5 * (unit_direction.y + 1.0);
        pixel_color = vec3_add(
            vec3_scale(vec3_make(1.0, 1.0, 1.0), 1.0 - t),
            vec3_scale(vec3_make(0.5, 0.7, 1.0), t));
    }

    for (int i = reflection_depth - 1; i >= 0; --i)
    {
        hit_record rec = hit_records[i];
        switch (rec.en)
        {
        case SPHERE_METAL:
            pixel_color = color_transform_sphere_metal(rec.entity, rec, pixel_color, state);
            break;
        case SPHERE_LAMBERTIAN:
            pixel_color = color_transform_sphere_lambertian(rec.entity, rec, pixel_color, state);
            break;
        case TRIANGLE_METAL:
            pixel_color = color_transform_triangle_metal(rec.entity, rec, pixel_color, state);
            break;
        case TRIANGLE_LAMBERTIAN:
            pixel_color = color_transform_triangle_lambertian(rec.entity, rec, pixel_color, state);
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
    int global_seed = rand();

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

                double d = rand();

                vec3 direction = vec3_add(
                    LOWER_LEFT_CORNER,
                    vec3_add(
                        vec3_scale(HORIZONTAL, u),
                        vec3_scale(VERTICAL, v)));

                ray r = ray_make(CAMERA_ORIGIN, direction);
                col = vec3_add(col, ray_color(r, SPHERES, NUM_OF_SPHERES, TRIANGLES, NUM_OF_TRIANGLES, &global_seed));
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

int main(void)
{
    setup_scene();
    color image[HEIGHT][WIDTH];
    render(image);
    save_ppm("sphere.ppm", image);
    return 0;
}
