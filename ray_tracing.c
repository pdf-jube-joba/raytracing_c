#include <stdio.h>
#include <sys/time.h> // for time

#include "vec3.h"
#include "world_entity.h"
#include "scene.h"

color ray_color(ray r, unsigned int *state)
{
    material hit_mat[MAX_REFLECTION_DEPTH];

    int reflection_depth = 0;

    for (reflection_depth = 0; reflection_depth < MAX_REFLECTION_DEPTH; ++reflection_depth)
    {
        hit_record_geometry closest = {.t = -1.0};

        for (size_t i = 0; i < ENTITY_NUM; ++i)
        {
            hit_record_geometry rec = hit_geometry(ENTITY[i].geo, r);
            if (hit_record_closer(&closest, rec))
            {
                hit_mat[reflection_depth] = ENTITY[i].mat;
            }
        }

        if (closest.t < 0.0)
        {
            // no hit
            break;
        }
        else
        {
            // hit
            r = scatter_material(hit_mat[reflection_depth], closest, state);
        }
    }

    color pixel_color = background_color(r);

    for (int i = reflection_depth - 1; i >= 0; --i)
    {
        pixel_color = color_transform_material(hit_mat[i], pixel_color, state);
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

    save_ppm("ri.ppm", image);
    return 0;
}
