#include "Spark/core/clock.h"
#include "Spark/core/smemory.h"
#include "Spark/defines.h"
#include "Spark/core/logging.h"

#include "Spark/entry.h"
#include "Spark/math/smath.h"
#include "Spark/memory/dynamic_allocator.h"
#include <stdlib.h>

// =========================
// CONFIG
// =========================
#define RAND_ALLOC_COUNT 1000
const u32 alloc_size_count = 7;
const u32 alloc_sizes[] = {
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    2048,
    4096
};
const u32 test_count = 300;

// =========================
// STATE
// =========================
static u32 allocation_size = 1024;
u32 rand_vector[RAND_ALLOC_COUNT];
u32* rand_alloc_ints[RAND_ALLOC_COUNT];

static u64 allocation_total = 0;
static dynamic_allocator_t allocator;

SINLINE void freelist_benchmark();
SINLINE void freelist_randalloc_benchmark();
SINLINE void malloc_benchmark();
SINLINE void malloc_randalloc_benchmark();

int sort_median_times(const void* a, const void* b) {
    f64 _a = *(f64*)a;
    f64 _b = *(f64*)b;
    if (_a < _b) {
        return -1;
    } else if (_a > _b) {
        return 1;
    } else { 
        return 0;
    }
}

b8 create_game(game_t *out_game) {
    return true;
}

double run_benchmark(void (*benchmark_function)(), u32 iteration_count, const char* name) {
    spark_clock_t clock;
    clock_start(&clock);
    benchmark_function();
    clock_update(&clock);
    // STRACE("[%s] Single call: %fms", name, clock.elapsed_time * 1000);
    clock_start(&clock);
    for (u32 i = 0; i < iteration_count; i++) {
        benchmark_function();
    }
    clock_update(&clock);
    double iteration_time = clock.elapsed_time * 1000 * 1000;
    // STRACE("[%s] %d calls: %fms (Avg: %.10fms)", name, iteration_count, iteration_time, clock.elapsed_time * 1000 / iteration_count);
    return iteration_time;
}

void init_random_vector() {
    // Setup initial values
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        rand_vector[i] = i;
    }
    
    // for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
    //     int index = random() % RAND_ALLOC_COUNT;
    //
    //     u32 temp = rand_vector[index];
    //     rand_vector[index] = rand_vector[i];
    //     rand_vector[i] = temp;
    // }
}

s32 main(s32 argc, char** argv) {
    // Setup
    dynamic_allocator_create(512 * MB, &allocator);
    init_random_vector();

    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        b8 found = false;
        for (u32 j = 0; j < RAND_ALLOC_COUNT; j++) {
            if (rand_vector[j] == i) {
                found = true;
            }
        }
        SASSERT(found, "Failed to correctly initialize random vector. Could not find value %d", i);
    }

#define BENCHMARK_FUNCTION_COUNT 4
    u32 iteration_counts[BENCHMARK_FUNCTION_COUNT] = {
        50000,
        500,
        50000,
        500,
    };

    void (*benchmark_functoins[BENCHMARK_FUNCTION_COUNT])() = {
        freelist_benchmark,
        freelist_randalloc_benchmark,
        malloc_benchmark,
        malloc_randalloc_benchmark,
    };

    const char* benchmark_function_names[BENCHMARK_FUNCTION_COUNT] = {
        "Freelist       ",
        "Freelist Random",
        "Malloc         ",
        "Malloc   Random",
    };


    for (u32 size = 0; size < alloc_size_count; size++) {
        allocation_size = alloc_sizes[size];
        STRACE("Starting Allocation size %d", allocation_size);
        double total_times[BENCHMARK_FUNCTION_COUNT] = {};
        double min_times[BENCHMARK_FUNCTION_COUNT] = {};
        double max_times[BENCHMARK_FUNCTION_COUNT] = {};

        for (u32 i = 0; i < BENCHMARK_FUNCTION_COUNT; i++) {
            min_times[i] = 1000000000;
            max_times[i] = -100000000;
        }

        f64 iteration_times[BENCHMARK_FUNCTION_COUNT][test_count];
        for (u32 i = 0; i < test_count; i++) {
            for (u32 f = 0; f < BENCHMARK_FUNCTION_COUNT; f++) {
                double time = run_benchmark(benchmark_functoins[f], iteration_counts[f], benchmark_function_names[f]);
                total_times[f] += time;
                min_times[f] = smin(min_times[f], time);
                max_times[f] = smax(max_times[f], time);

                iteration_times[f][i] = time;
            }

            // freelist_print_debug_allocations(&allocator);
        }

        for (u32 i = 0; i < BENCHMARK_FUNCTION_COUNT; i++) {
            qsort(iteration_times[i], test_count, sizeof(f64), sort_median_times);
            SINFO("[%s]     Time: %fus", benchmark_function_names[i], total_times[i]);
            SINFO("[%s] Avg time: %fus", benchmark_function_names[i], total_times[i] / test_count / iteration_counts[i]);
            SINFO("[%s] Min time: %fus", benchmark_function_names[i], min_times[i] / iteration_counts[i]);
            SINFO("[%s] Max time: %fus", benchmark_function_names[i], max_times[i] / iteration_counts[i]);
            SINFO("[%s] Med time: %fus", benchmark_function_names[i], iteration_times[i][test_count / 2] / iteration_counts[i]);
        }
    }
}

SINLINE void malloc_benchmark() {
    void* value = malloc(allocation_size);
    allocation_total += (u64)value;
    free(value);
}

SINLINE void freelist_benchmark() {
    void* value = dynamic_allocator_allocate(&allocator, allocation_size);
    allocation_total += (u64)value;
    dynamic_allocator_free(&allocator, value);
}

SINLINE void malloc_randalloc_benchmark() {
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        rand_alloc_ints[i] = malloc(allocation_size);
    }
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        free(rand_alloc_ints[rand_vector[i]]);
    }
}

SINLINE void freelist_randalloc_benchmark() {
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        rand_alloc_ints[i] = dynamic_allocator_allocate(&allocator, allocation_size);
    }
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        dynamic_allocator_free(&allocator, rand_alloc_ints[rand_vector[i]]);
    }
}
