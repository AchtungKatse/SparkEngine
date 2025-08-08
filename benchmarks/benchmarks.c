#include "Spark/core/clock.h"
#include "Spark/core/smemory.h"
#include "Spark/defines.h"
#include "Spark/core/logging.h"

#include "Spark/containers/unordered_map.h"
#include "Spark/memory/freelist.h"
#include "Spark/utils/hashing.h"
#include <stdlib.h>

#define ALLOCATION_SIZE (10 * MB)

static u64 allocation_total = 0;
static freelist_t allocator;
SINLINE void general_allocator_benchmark();
SINLINE void malloc_benchmark();

#define run_benchmark(benchmark_function, iteration_count, name) \
{ \
    spark_clock_t clock; \
    clock_start(&clock); \
    benchmark_function(); \
    clock_update(&clock); \
    SINFO("[" name "] Single call: %fms", clock.elapsed_time * 1000); \
    clock_start(&clock); \
    for (u32 i = 0; i < iteration_count; i++) { \
        benchmark_function(); \
    } \
    clock_update(&clock); \
    SINFO("[" name "] %d calls: %fms (Avg: %.10fms)", iteration_count, clock.elapsed_time * 1000, clock.elapsed_time * 1000 / iteration_count); \
}

s32 main(s32 argc, char** argv) {
    // Setup
    const u32 iteration_count = 1000000000;
    freelist_create(256 * MB, false, &allocator);

    // Call genearal_allocator_allocate once to get it into cache since the first call will always be in ram
    // This levels the playing field between this and malloc
    void* a = freelist_allocate(&allocator, ALLOCATION_SIZE, MEMORY_TAG_ARRAY);
    freelist_free(&allocator, a);
    void* b = malloc(ALLOCATION_SIZE);
    free(b);

    run_benchmark(malloc_benchmark, iteration_count, "Malloc");
    run_benchmark(general_allocator_benchmark, iteration_count, "General Allocator");
}

SINLINE void malloc_benchmark() {
    void* value = malloc(ALLOCATION_SIZE);
    allocation_total += (u64)value;
    free(value);
}

SINLINE void general_allocator_benchmark() {
    void* value = freelist_allocate(&allocator, ALLOCATION_SIZE, MEMORY_TAG_ARRAY);
    allocation_total += (u64)value;
    freelist_free(&allocator, value);
}

