#pragma once

#include "Spark/defines.h"

typedef struct linear_allocator {
    u64 total_size;
    u64 allocated;
    void* memory;
    b8 owns_memory;
} linear_allocator_t;

SAPI void linear_allocator_create(u64 total_size, void* memory, linear_allocator_t* out_allocator);
SAPI void linear_allocator_destroy(linear_allocator_t* allocator);

SAPI void* linear_allocator_allocate(linear_allocator_t* allocator, u64 size);
SAPI void linear_allocator_free_all(linear_allocator_t* allocator);
