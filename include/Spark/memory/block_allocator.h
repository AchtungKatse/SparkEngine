#pragma once

typedef struct block_allocator_block {
    struct block_allocator_block* next;
} block_allocator_block_t;

typedef struct block_allocator {
    void* buffer;
    block_allocator_block_t* blocks;
    u32 buffer_size;
} block_allocator_t;

void block_allocator_create(u32 block_count, u32 block_size, block_allocator_t* out_allocator);
void block_allocator_destroy(u32 block_count, u32 block_size, block_allocator_t* out_allocator);

void* block_allocator_allocate(block_allocator_t* allocator);
void block_allocator_free(block_allocator_t* allocator, void* block);

