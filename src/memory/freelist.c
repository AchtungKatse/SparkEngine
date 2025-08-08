#include "Spark/core/smemory.h"
#include "Spark/memory/freelist.h"
#include "Spark/platform/platform.h"
#include "Spark/math/smath.h"
#include <string.h>

// typedef struct freelist_block {
//     u32 size;
//     b32 allocated;
// } freelist_block_t;
//
void freelist_create(u64 size, b8 aligned, freelist_t* out_allocator) {
    out_allocator->memory_size    = size + sizeof(freelist_block_t) * 4;
    out_allocator->memory         = platform_allocate(out_allocator->memory_size, aligned);
    out_allocator->aligned        = aligned;
    out_allocator->next_allocator = NULL;

    // Create first block
    freelist_block_t* block = out_allocator->memory + sizeof(freelist_block_t) * 2;
    block->size             = size;
    block->allocated        = false;

    out_allocator->first_block  = block;

    // Set end of memory block
    freelist_block_t* first_block = out_allocator->memory + sizeof(freelist_block_t);
    first_block->size = 0;
    first_block->allocated = true;

    freelist_block_t* last_block = out_allocator->memory + size + sizeof(freelist_block_t);
    last_block->size = 0;
    last_block->allocated = true;
    // SDEBUG("Last block addr: %p", size + sizeof(freelist_block_t));
}

void freelist_destroy(freelist_t* allocator) {
    platform_free(allocator->memory, allocator->aligned);
}

void* freelist_allocate(freelist_t* allocator, u64 size, memory_tag_t tag) {
    SASSERT(size > 0, "Cannot allocate nothing.");

    // Find first block with valid size
    freelist_block_t* block = allocator->first_block;

    u32 required_size = size + sizeof(freelist_block_t); // Add freelist_block_t size for footer block
    while (block) {
        if (block->size == 0) {
            break;
        }

        // Check if block can contain allocation
        if (block->size < required_size) {
            block = (void*)block + block->size + sizeof(freelist_block_t) * 2;
            continue;
        }

        // It can
        // Split the block into the allocation and a new block
        const u32 remaining_size = block->size - size - sizeof(freelist_block_t);
        if (sizeof(freelist_block_t) < remaining_size) {
            freelist_block_t* new_block = ((void*)block) + sizeof(freelist_block_t) * 2 + size;
            new_block->size = remaining_size - sizeof(freelist_block_t);
        }

        // Create a new allocation
        freelist_block_t* block_end = ((void*)block) + sizeof(freelist_block_t) + size;
        block_end->size      = size;
        block_end->allocated = true;

        block->size          = size;
        block->allocated     = true;

        return ((void*)block) + sizeof(freelist_block_t);
    }

    // Failed to allocate, check sub allocator
    if (allocator->next_allocator) {
        SWARN("Suballocator allocation. Size: %d", size);
        return freelist_allocate(allocator->next_allocator, size, tag);
    }

    // Failed to allocate the data, Create a new sub-allocator
    SWARN("Creating sub-allocator");
    allocator->next_allocator = platform_allocate(sizeof(freelist_t), allocator->aligned);
    freelist_create(smin(size * 4, GENERAL_ALLOCATOR_DEFAULT_SIZE), allocator->aligned, allocator->next_allocator);
    return freelist_allocate(allocator->next_allocator, size, tag);
}

void freelist_free(freelist_t* allocator, void* address) {
    // get allocation
    freelist_block_t* block = address - sizeof(freelist_block_t);
    freelist_block_t* block_header = address + block->size;
    SASSERT(block->allocated, "Cannot free unallocated allocation.");
    // SDEBUG("\n\nFreeing: 0x%x", ((void*)block - allocator->memory) / 0x30);

    // Convert allocation to block
    enum coalesce_state {
        coalesce_state_default = 0,
        coalesce_state_previous = 1,
        coalesce_state_next = 2,
        coalesce_state_all = 3,
    };

    freelist_block_t* previous_block = address - sizeof(freelist_block_t) * 2;
    freelist_block_t* next_block = address + block->size + sizeof(freelist_block_t);

    // SDEBUG("Block          : Addr: %p, Size: 0x%x, Allocated: %d", (void*)block - allocator->memory, block->size, block->allocated);
    // SDEBUG("Initial Offsets: %p, %p", (void*)block - allocator->memory, (void*)block_header - allocator->memory + sizeof(freelist_block_t));
    // SDEBUG("Previous Block : Addr: %p, Size: 0x%x, Allocated: %d", (void*)previous_block - allocator->memory, previous_block->size, previous_block->allocated);
    // SDEBUG("Next Block     : Addr: %p, Size: 0x%x, Allocated: %d", (void*)next_block - allocator->memory, next_block->size, next_block->allocated);

    enum coalesce_state state = 0;
    if (!previous_block->allocated && block != allocator->memory) {
        state |= coalesce_state_previous;
    }

    if (!next_block->allocated) {
        state |= coalesce_state_next;
    }

    switch (state) {
        case coalesce_state_default:
            // SDEBUG("Coalesce default");
            block->allocated = false;
            block_header->allocated = false;
            break;
        case coalesce_state_previous:
            // SDEBUG("Coalesce previous");
            previous_block = (void*)previous_block - previous_block->size - sizeof(freelist_block_t);
            previous_block->size += block->size + sizeof(freelist_block_t) * 2;
            block = previous_block;
            block_header->size = block->size;
            block_header->allocated = false;
            break;
        case coalesce_state_next:
            // SDEBUG("Coalesce next");
            block->allocated = false;
            block->size += next_block->size + sizeof(freelist_block_t) * 2;
            block_header = (void*)block_header + next_block->size + sizeof(freelist_block_t) * 2;
            block_header->size = block->size;
            block_header->allocated = false;
            break;
        case coalesce_state_all:
            // SDEBUG("Coalesce all");
            previous_block = (void*)previous_block - previous_block->size - sizeof(freelist_block_t);
            previous_block->size += block->size + next_block->size + sizeof(freelist_block_t) * 4;
            block = previous_block;
            block_header = (void*)block_header + next_block->size + sizeof(freelist_block_t) * 2;
            block_header->size = block->size;
            block_header->allocated = false;
            break;
    }

    // SDEBUG("New Size: 0x%x / 0x%x", block->size, ((void*)block_header - allocator->memory + sizeof(freelist_block_t)) - ((void*)block - allocator->memory) - sizeof(freelist_block_t) * 2);
    // SDEBUG("New Offsets: %p, %p", (void*)block - allocator->memory, (void*)block_header - allocator->memory + sizeof(freelist_block_t));
}

