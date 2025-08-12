#pragma once

#include "Spark/memory/freelist.h"
typedef struct dynamic_allocator {
    struct dynamic_allocator* next_allocator;
    freelist_t freelist;
    u64 memory_size;
} dynamic_allocator_t;

void dynamic_allocator_create(u64 memory_size, dynamic_allocator_t* out_allocator);
void dynamic_allocator_destroy(dynamic_allocator_t* allocator);

void* dynamic_allocator_allocate(dynamic_allocator_t* allocator, u64 size);
void dynamic_allocator_free(dynamic_allocator_t* allocator, void* data);
