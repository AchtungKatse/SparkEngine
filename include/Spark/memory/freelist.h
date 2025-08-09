#pragma once

#include "Spark/core/smemory.h"

#define GENERAL_ALLOCATOR_DEFAULT_SIZE     (256 * MB)

typedef struct freelist_block {
    u32 size;
    b32 allocated;
} freelist_block_t;

struct freelist_explicit;

typedef struct freelist {
    void* memory;
    u64 memory_size;
    struct freelist_block* first_block;
    struct freelist_explicit* first_free_block;
    struct freelist* next_allocator;
    b8 aligned;
} freelist_t;

void freelist_create(u64 size, b8 alligned, freelist_t* out_allocator);
void freelist_destroy(freelist_t* allocator);

void* freelist_allocate(freelist_t* allocator, u64 size);
void freelist_free(freelist_t* allocator, void* address);
void freelist_print_debug_allocations(freelist_t* allocator);
