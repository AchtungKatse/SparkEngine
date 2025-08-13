#include "Spark/core/clock.h"
#include "Spark/entry.h"
#include "Spark/math/math_types.h"
#include "Spark/random/noise/simplex.h"

#define ITERATION_COUNT 1000
#define SAMPLE_COUNT_2D 1000

#define CHUNK_SIZE 32
#define POINTS_PER_CHUNK (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define OCTAVE_COUNT 8

void benchmark(const char* name, void (function())) {
    SINFO("Starting: %s", name);
    spark_clock_t clock; 
    clock_start(&clock);
    function();
    clock_update(&clock);
    const u64 point_count = ITERATION_COUNT * SAMPLE_COUNT_2D * SAMPLE_COUNT_2D;
    f64 time_per_point_us = clock.elapsed_time * 1000 * 1000 / point_count;
    SINFO("%s Time: %fms (%d points; %fus / point)", name, clock.elapsed_time * 1000, point_count, time_per_point_us);
    for (u32 i = 1; i <= OCTAVE_COUNT; i++) {
        f64 time_per_chunk = POINTS_PER_CHUNK * i * time_per_point_us / 1000;
        SINFO("\tEstimated time for %d octaves: %fms (%f chunks/sec)", i, time_per_chunk, 1.0f / time_per_chunk * 1000);
    }
}

void benchmark_simplex_2d();
void benchmark_simplex_2d_int();

b8 create_game(game_t *out_game) {
    return true;
}

s32 main(s32 argc, const char** argv) {
    for (u32 i = 1; i <= OCTAVE_COUNT; i++) {
        SINFO("Estimated points per chunk at %d octaves: %d", i, POINTS_PER_CHUNK * i);
    }

    benchmark("Simplex Float", benchmark_simplex_2d);
    benchmark("Simplex Int  ", benchmark_simplex_2d_int);

    return 0;
}

void benchmark_simplex_2d() {
    for (u32 i = 0; i < ITERATION_COUNT; i++) {
        for (u32 x = 0; x < SAMPLE_COUNT_2D; x++) {
            for (u32 y = 0; y < SAMPLE_COUNT_2D; y++) {
                vec2 pos = {
                    .x = x * .01f,
                    .y = y * .01f,
                };
                simplex_2d(pos);
            }
        }
    }
}

void benchmark_simplex_2d_int() {
    for (u32 i = 0; i < ITERATION_COUNT; i++) {
        for (u32 x = 0; x < SAMPLE_COUNT_2D; x++) {
            for (u32 y = 0; y < SAMPLE_COUNT_2D; y++) {
                vec2i pos = {
                    .x = x * (s32)(65535 * .01f),
                    .y = y * (s32)(65535 * .01f),
                };
                simplex_2d_int(pos);
            }
        }
    }
}
