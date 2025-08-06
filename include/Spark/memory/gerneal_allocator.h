#pragma once

#include "Spark/core/smemory.h"

#define GENERAL_ALLOCATOR_DEFAULT_SIZE     256 * MB
#define GENERAL_ALLOCATOR_BLOCK_MAGIC      0x47414C43
#define GENERAL_ALLOCATOR_ALLOCATION_MAGIC 0x47424C4B

typedef struct general_allocator_block {
    struct general_allocator_block* next_block;
    struct general_allocator_block* previous_block;
    u32 magic;
    u32 size;
} general_allocator_block_t;

typedef struct general_allocator {
    void* memory;
    u64 memory_size;
    general_allocator_block_t* first_block;
    struct general_allocator* next_allocator;
    b8 aligned;
} general_allocator_t;

void general_allocator_create(u64 size, b8 alligned, general_allocator_t* out_allocator);
void general_allocator_destroy(general_allocator_t* allocator);

void* general_allocator_allocate(general_allocator_t* allocator, u64 size, memory_tag_t tag);
void general_allocator_free(general_allocator_t* allocator, void* address);
