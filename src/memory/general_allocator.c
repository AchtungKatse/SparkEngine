#include "Spark/core/smemory.h"
#include "Spark/memory/gerneal_allocator.h"
#include "Spark/platform/platform.h"
#include "Spark/math/smath.h"

void general_allocator_create(u64 size, b8 aligned, general_allocator_t* out_allocator) {
    out_allocator->memory_size = size + sizeof(general_allocator_block_t);
    out_allocator->memory = platform_allocate(size, aligned);
    out_allocator->aligned = aligned;

    // Create first block
    general_allocator_block_t* block = out_allocator->memory;
    block->size = size;
    block->magic = GENERAL_ALLOCATOR_BLOCK_MAGIC;
    block->next_block = NULL;
    out_allocator->first_block = block;
}

void general_allocator_destroy(general_allocator_t* allocator) {
    platform_free(allocator->memory, allocator->aligned);
}

void* general_allocator_allocate(general_allocator_t* allocator, u64 size, memory_tag_t tag) {
    // SASSERT(size > 0, "Cannot allocate nothing.");

    // Find first block with valid size
    general_allocator_block_t* block = allocator->first_block;
    general_allocator_block_t** parent_block = &allocator->first_block;

    while (block) {
        // SASSERT((void*)block >= allocator->memory && (void*)block < allocator->memory + allocator->memory_size,
        //         "Invalid general allocator block address. %p <= %p < %p", allocator->memory, block, allocator->memory + allocator->memory_size);
        // Check if usable block
        const u32 block_size = block->size;

        if (size > block_size) {
            parent_block = &block->next_block;
            // SASSERT(block->next_block == NULL || ((void*)block->next_block >= allocator->memory && (void*)block->next_block < allocator->memory + allocator->memory_size),
            //         "Invalid general allocator block address. %p <= %p < %p", allocator->memory, block, allocator->memory + allocator->memory_size);
            block = block->next_block;
            continue;
        }

        // Mark parent block pointer to new block
        u64 remaining_size = block_size - size;
        if (remaining_size >= sizeof(general_allocator_block_t)) {
            general_allocator_block_t* new_block = ((void*)block) + sizeof(general_allocator_block_t) + size;
            new_block->size = remaining_size - sizeof(general_allocator_block_t);
            new_block->magic = GENERAL_ALLOCATOR_BLOCK_MAGIC;
            new_block->next_block = block->next_block;
            new_block->previous_block = block;
            *parent_block = new_block;
        }

        // Create new allocation
        // Override the block with an allocation
        general_allocator_block_t* allocation = (general_allocator_block_t*)block;
        allocation->magic = GENERAL_ALLOCATOR_ALLOCATION_MAGIC;
        allocation->size = size;

        return ((void*)allocation) + sizeof(general_allocator_block_t);

    }

    // Failed to allocate, check sub allocator
    if (allocator->next_allocator) {
        // SWARN("Suballocator allocation");
        return general_allocator_allocate(allocator->next_allocator, size, tag);
    }

    // Failed to allocate the data, Create a new sub-allocator
        // SWARN("Creating sub-allocator");
    allocator->next_allocator = platform_allocate(sizeof(general_allocator_t), allocator->aligned);
    general_allocator_create(smin(size * 4, GENERAL_ALLOCATOR_DEFAULT_SIZE), allocator->aligned, allocator->next_allocator);
    return general_allocator_allocate(allocator->next_allocator, size, tag);
}

void general_allocator_free(general_allocator_t* allocator, void* address) {
    // get allocation
    general_allocator_block_t* allocation = address - sizeof(general_allocator_block_t);
    // SASSERT(allocation->magic == GENERAL_ALLOCATOR_ALLOCATION_MAGIC, "Cannot free genera allocator object. Expected magic of 0x%x, got 0x%x", GENERAL_ALLOCATOR_ALLOCATION_MAGIC, allocation->magic);

    // Convert allocation to block
    general_allocator_block_t* block = (general_allocator_block_t*)allocation; 
    u64 new_size = allocation->size + sizeof(general_allocator_block_t) - sizeof(general_allocator_block_t);
    block->size = new_size;
    block->magic = GENERAL_ALLOCATOR_BLOCK_MAGIC;

    // Defragment blocks
    general_allocator_block_t* future_block = address + allocation->size;
    if (future_block->magic == GENERAL_ALLOCATOR_BLOCK_MAGIC) {
        if (future_block->previous_block) {
            block->previous_block = future_block->previous_block;
            block->previous_block->next_block = block;
            block->size += sizeof(general_allocator_block_t) + future_block->size;
        }
    }


    block->next_block = allocator->first_block;
    allocator->first_block = block;
}

