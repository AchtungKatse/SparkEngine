#include "Spark/core/clock.h"
#include "Spark/entry.h"
#include "Spark/math/math_types.h"
#include "Spark/random/noise/simplex.h"

#define UNMARKED_ITERATION_COUNT 10
#define ITERATION_COUNT 8000
#define SAMPLE_COUNT_2D 512
#define SAMPLE_COUNT_3D 100

#define CHUNK_SIZE 32
#define POINTS_PER_CHUNK (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define OCTAVE_COUNT 8

// Benchmark Results
// Baseline
// [INFO]:  Simplex Float Time: 1802.180373ms (200000000 points; 0.009011us / point)
// [INFO]:  Simplex Int   Time: 1267.946254ms (200000000 points; 0.006340us / point)
//
// Reduced Memory Size
// [INFO]:  Simplex Float Time: 1796.696719ms (200000000 points; 0.008983us / point)
// [INFO]:  Simplex Int   Time: 1214.831757ms (200000000 points; 0.006074us / point)

static float noise[SAMPLE_COUNT_2D * SAMPLE_COUNT_2D];
static s32 out_noise[SAMPLE_COUNT_3D * SAMPLE_COUNT_3D * SAMPLE_COUNT_3D];

void benchmark(const char* name, void (function()), const u64 point_count) {
    SINFO("Starting: %s", name);
    spark_clock_t clock; 
    clock_start(&clock);
    function();
    clock_update(&clock);
    f64 time_per_point_us = clock.elapsed_time * 1000 * 1000 / point_count;
    SINFO("%s Time: %fms (%d points; %fus / point; %f points/s)", name, clock.elapsed_time * 1000, point_count, time_per_point_us, 1.0 / time_per_point_us * 1000 * 1000);
    f64 time_per_chunk = POINTS_PER_CHUNK * OCTAVE_COUNT * time_per_point_us / 1000;
    SINFO("\tEstimated time for %d octaves: %fms (%f chunks/sec)", OCTAVE_COUNT, time_per_chunk, 1.0f / time_per_chunk * 1000);

    f32 noise_total = 0;
    for (u32 i = 0; i < SAMPLE_COUNT_2D * SAMPLE_COUNT_2D; i++) {
        noise_total += noise[i];
    }
    for (u32 i = 0; i < SAMPLE_COUNT_3D * SAMPLE_COUNT_3D * SAMPLE_COUNT_3D; i++) {
        noise_total += out_noise[i];
    }
    SINFO("Noise total: %f", noise_total);
}

void benchmark_simplex_2d();
void benchmark_simplex_2d_int();
void benchmark_simplex_2d_int_simd();

void benchmark_simplex_3d();
void benchmark_simplex_3d_int();

b8 create_game(game_t *out_game) {
    return true;
}

s32 main(s32 argc, const char** argv) {
    u32 magic_number = 5462;
    const u64 point_count_2d = ITERATION_COUNT * SAMPLE_COUNT_2D * SAMPLE_COUNT_2D;
    // const u64 point_count_3d = ITERATION_COUNT * SAMPLE_COUNT_3D * SAMPLE_COUNT_3D * SAMPLE_COUNT_3D;
    // benchmark("Simplex 2D Float   ", benchmark_simplex_2d,          point_count_2d);
    // benchmark("Simplex 2D Int     ", benchmark_simplex_2d_int,      point_count_2d);
    for (u32 i = 0; i < UNMARKED_ITERATION_COUNT; i++) {
        benchmark("Simplex 2D Int SIMD", benchmark_simplex_2d_int_simd, point_count_2d);
    }

    // benchmark("Simplex 3D Float   ", benchmark_simplex_3d,          point_count_3d);
    // benchmark("Simplex 3D Int     ", benchmark_simplex_3d_int,      point_count_3d);

    return 0;
}

void benchmark_simplex_2d() {
    for (u32 i = 0; i < ITERATION_COUNT; i++) {
        for (u32 x = 0, s = 0; x < SAMPLE_COUNT_2D; x++) {
            for (u32 y = 0; y < SAMPLE_COUNT_2D; y++, s++) {
                vec2 pos = {
                    .x = x * .01f,
                    .y = y * .01f,
                };
                noise[s] = simplex_2d(0, pos);
            }
        }
    }
}

void benchmark_simplex_2d_int() {
    for (u32 i = 0; i < ITERATION_COUNT; i++) {
        for (u32 x = 0, s = 0; x < SAMPLE_COUNT_2D; x++) {
            for (u32 y = 0; y < SAMPLE_COUNT_2D; y++, s++) {
                vec2i pos = {
                    .x = x * (s32)(65535 * .01f),
                    .y = y * (s32)(65535 * .01f),
                };
                noise[s] = simplex_2d_int(0, pos);
            }
        }
    }
}

void benchmark_simplex_2d_int_simd() {
    for (u32 i = 0; i < ITERATION_COUNT; i++) {
        vec2i pos = {
            .x = 0,
            .y = 0,
        };
        simplex_2d_int_simd(0, pos, (vec2i) {.x = SAMPLE_COUNT_2D, .y = SAMPLE_COUNT_2D}, 128, out_noise);
    }
}

// void benchmark_simplex_3d() {
//     for (u32 i = 0; i < ITERATION_COUNT; i++) {
//         for (u32 x = 0; x < SAMPLE_COUNT_3D; x++) {
//             for (u32 y = 0; y < SAMPLE_COUNT_3D; y++) {
//                 for (u32 z = 0; z < SAMPLE_COUNT_3D; z++) {
//                     vec3 pos = {
//                         .x = x * .01f,
//                         .y = y * .01f,
//                         .z = z * .01f,
//                     };
//                     simplex_3d(pos);
//                 }
//             }
//         }
//     }
// }
//
// void benchmark_simplex_3d_int() {
//     for (u32 i = 0; i < ITERATION_COUNT; i++) {
//         for (u32 x = 0; x < SAMPLE_COUNT_3D; x++) {
//             for (u32 y = 0; y < SAMPLE_COUNT_3D; y++) {
//                 for (u32 z = 0; z < SAMPLE_COUNT_3D; z++) {
//                     vec3i pos = {
//                         .x = (s32)(65535 * x * .01f),
//                         .y = (s32)(65535 * y * .01f),
//                         .z = (s32)(65535 * z * .01f),
//                     };
//                     simplex_3d_int(pos);
//                 }
//             }
//         }
//     }
// }
