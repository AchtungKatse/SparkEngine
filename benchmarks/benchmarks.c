#include "Spark/core/clock.h"
#include "Spark/core/smemory.h"
#include "Spark/defines.h"
#include "Spark/core/logging.h"

#include "Spark/containers/unordered_map.h"
#include "Spark/math/smath.h"
#include "Spark/memory/freelist.h"
#include "Spark/utils/hashing.h"
#include <stdlib.h>

#define ALLOCATION_SIZE (16)
#define RAND_ALLOC_COUNT 2000

u32 rand_vector[RAND_ALLOC_COUNT];

static u64 allocation_total = 0;
static freelist_t allocator;

SINLINE void freelist_benchmark();
SINLINE void freelist_randalloc_benchmark();
SINLINE void malloc_benchmark();
SINLINE void malloc_randalloc_benchmark();

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
    double iteration_time = clock.elapsed_time * 1000;
    // STRACE("[%s] %d calls: %fms (Avg: %.10fms)", name, iteration_count, iteration_time, clock.elapsed_time * 1000 / iteration_count);
    return iteration_time;
}

void init_random_vector() {
    // Setup initial values
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        rand_vector[i] = i;
    }
    
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        int index = random() % RAND_ALLOC_COUNT;

        u32 temp = rand_vector[index];
        rand_vector[index] = rand_vector[i];
        rand_vector[i] = temp;
    }
}

s32 main(s32 argc, char** argv) {
    // Setup
    freelist_create(RAND_ALLOC_COUNT * ALLOCATION_SIZE * 1600, false, &allocator);
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
        1000000,
        10000,
        1000000,
        10000,
    };

    void (*benchmark_functoins[BENCHMARK_FUNCTION_COUNT])() = {
        malloc_benchmark,
        malloc_randalloc_benchmark,
        freelist_benchmark,
        freelist_randalloc_benchmark,
    };

    const char* benchmark_function_names[BENCHMARK_FUNCTION_COUNT] = {
        "Malloc         ",
        "Malloc   Random",
        "Freelist       ",
        "Freelist Random",
    };

    double total_times[BENCHMARK_FUNCTION_COUNT] = {};
    double min_times[BENCHMARK_FUNCTION_COUNT] = {};
    double max_times[BENCHMARK_FUNCTION_COUNT] = {};

    for (u32 i = 0; i < BENCHMARK_FUNCTION_COUNT; i++) {
        min_times[i] = 1000000000;
        max_times[i] = -100000000;
    }

    STRACE("Starting...");
    const u32 test_count = 1;
    for (u32 i = 0; i < test_count; i++) {
        for (u32 f = 0; f < BENCHMARK_FUNCTION_COUNT; f++) {
            double time = run_benchmark(benchmark_functoins[f], iteration_counts[f], benchmark_function_names[f]);
            total_times[f] += time;
            min_times[f] = smin(min_times[f], time);
            max_times[f] = smax(max_times[f], time);
        }
    }

    for (u32 i = 0; i < BENCHMARK_FUNCTION_COUNT; i++) {
        SINFO("[%s] Avg time: %fms", benchmark_function_names[i], total_times[i] / test_count);
        SINFO("[%s] Min time: %fms", benchmark_function_names[i], min_times[i]);
        SINFO("[%s] Max time: %fms", benchmark_function_names[i], max_times[i]);
    }
}

SINLINE void malloc_benchmark() {
    void* value = malloc(ALLOCATION_SIZE);
    allocation_total += (u64)value;
    free(value);
}

SINLINE void freelist_benchmark() {
    void* value = freelist_allocate(&allocator, ALLOCATION_SIZE);
    allocation_total += (u64)value;
    freelist_free(&allocator, value);
}

SINLINE void malloc_randalloc_benchmark() {
    u32* ints[RAND_ALLOC_COUNT];
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        ints[i] = malloc(ALLOCATION_SIZE);
    }
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        free(ints[rand_vector[i]]);
    }
}

SINLINE void freelist_randalloc_benchmark() {
    u32* ints[RAND_ALLOC_COUNT];
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        ints[i] = freelist_allocate(&allocator, ALLOCATION_SIZE);
    }
    for (u32 i = 0; i < RAND_ALLOC_COUNT; i++) {
        freelist_free(&allocator, ints[rand_vector[i]]);
    }
}
