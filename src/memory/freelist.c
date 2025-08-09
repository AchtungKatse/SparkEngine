#include "Spark/core/smemory.h"
#include "Spark/memory/freelist.h"
#include "Spark/platform/platform.h"
#include "Spark/math/smath.h"
#include <string.h>

// typedef struct freelist_block {
//     u32 size;
//     b32 allocated;
// } freelist_block_t;

typedef struct freelist_explicit {
    struct freelist_explicit* next;
    struct freelist_explicit* previous;
} freelist_explicit_t;

void* freelist_suballocate(freelist_t* allocator, u64 size);

void freelist_create(u64 size, b8 aligned, freelist_t* out_allocator) {
    size = smax(size, 1024);

    out_allocator->memory_size    = size + sizeof(freelist_block_t) * 3;
    out_allocator->memory         = platform_allocate(out_allocator->memory_size, aligned);
    out_allocator->aligned        = aligned;
    out_allocator->next_allocator = NULL;

    // Create first block
    freelist_block_t* block = out_allocator->memory + sizeof(freelist_block_t);
    block->size             = size;
    block->allocated        = false;

    out_allocator->first_block  = block;

    freelist_explicit_t* explicit = (void*)block + sizeof(freelist_block_t);
    explicit->next     = NULL;
    explicit->previous = NULL;
    out_allocator->first_free_block  = explicit;

    // Set end of memory block
    freelist_block_t* leading_block = out_allocator->memory;
    leading_block->size = 0;
    leading_block->allocated = true;

    freelist_block_t* trailing_block = out_allocator->memory + out_allocator->memory_size - sizeof(freelist_block_t);
    trailing_block->size = 0;
    trailing_block->allocated = true;
    // SDEBUG("Last block addr: %p", size + sizeof(freelist_block_t));
}

void freelist_destroy(freelist_t* allocator) {
    platform_free(allocator->memory, allocator->aligned);
}

void* freelist_suballocate(freelist_t* allocator, u64 size) {
    // Failed to allocate, check sub allocator
    if (allocator->next_allocator) {
        SWARN("Suballocator allocation. Size: %d", size);
        return freelist_allocate(allocator->next_allocator, size);
    }

    // Failed to allocate the data, Create a new sub-allocator
    SWARN("Creating sub-allocator");
    allocator->next_allocator = platform_allocate(sizeof(freelist_t), allocator->aligned);
    freelist_create(smin(size * 4, GENERAL_ALLOCATOR_DEFAULT_SIZE), allocator->aligned, allocator->next_allocator);
    return freelist_allocate(allocator->next_allocator, size);
}

void* freelist_allocate(freelist_t* allocator, u64 size) {
    size = smax(size, sizeof(freelist_explicit_t));
    SASSERT(size > 0, "Cannot allocate nothing.");

    if (!allocator->first_free_block) {
        return freelist_suballocate(allocator, size);
    }

    // Find first block with valid size
    freelist_block_t* block = (void*)allocator->first_free_block - sizeof(freelist_block_t);

    u32 required_size = size + sizeof(freelist_block_t); // Add freelist_block_t size for footer block
    while (block->size) {
        freelist_explicit_t* explicit = (void*)block + sizeof(freelist_block_t);

        // Check if block can contain allocation
        if (block->size < required_size) {
            if (!explicit->next) {
                break;
            }
            block = (void*)explicit->next - sizeof(freelist_block_t);
            continue;
        }

        // It can
        // Split the block into the allocation and a new block
        const u32 remaining_size = block->size - size - sizeof(freelist_block_t);

        if (sizeof(freelist_block_t) < remaining_size) {
            freelist_block_t* new_block = ((void*)block) + sizeof(freelist_block_t) * 2 + size;
            freelist_explicit_t* new_explicit = (void*)new_block + sizeof(freelist_block_t);
            new_explicit->next     = 0;
            new_explicit->previous = 0;

            new_block->size = remaining_size - sizeof(freelist_block_t);

            if (explicit->next) {
                explicit->next->previous = new_explicit;
                new_explicit->next = explicit->next;
            }
            if (explicit->previous) {
                explicit->previous->next = new_explicit;
                new_explicit->previous = explicit->previous;
            }
            if (explicit == allocator->first_free_block) {
                allocator->first_free_block = new_explicit;
            }
        } else {
            size = remaining_size;
            allocator->first_free_block = NULL;
        }



        // Create a new allocation
        freelist_block_t* block_end = ((void*)block) + sizeof(freelist_block_t) + size;
        block_end->size      = size;
        block_end->allocated = true;

        block->size          = size;
        block->allocated     = true;

        return ((void*)block) + sizeof(freelist_block_t);
    }

    return freelist_suballocate(allocator, size);
}

void test_explicit_freelist(freelist_t* allocator) {
    freelist_explicit_t* explicit = allocator->first_free_block;

    int iterations = 0;
    int block_count = 1;
    while (explicit) {
        freelist_block_t* block = (void*)explicit - sizeof(freelist_block_t);

        freelist_block_t* _block = allocator->first_block;
        while (_block != block && _block && _block->size) {
            _block = (void*)_block + _block->size + sizeof(freelist_block_t) * 2;
            block_count++;
        }

        explicit = explicit->next;
        // SASSERT(block->size > 0,    "Failed to manage explicit freelist: Targeting enclosing blocks.");
        SASSERT(!block->allocated,  "Failed to manage explicit freelist: Expected free block.");
        SASSERT(iterations < 10000, "Failed to manage explicit freelist: Recursive freelist detected.");
        SASSERT(_block == block,    "Failed to manage explicit freelist: Target block is not in freelist. Total block count: %d", block_count);
        iterations++;
    }
}

void freelist_free(freelist_t* allocator, void* address) {
    while (address < allocator->memory || address > allocator->memory + allocator->memory_size) {
        allocator = allocator->next_allocator;
    }
    // get allocation
    freelist_block_t* block = address - sizeof(freelist_block_t);
    freelist_block_t* block_header = address + block->size;
    SASSERT(block->allocated, "Cannot free unallocated allocation.");

    // Convert allocation to block
    enum coalesce_state {
        coalesce_state_default = 0,
        coalesce_state_previous = 1,
        coalesce_state_next = 2,
        coalesce_state_all = 3,
    };

    freelist_explicit_t* new_explicit = address;
    freelist_block_t* previous_block = address - sizeof(freelist_block_t) * 2;
    freelist_explicit_t* previous_explicit = (void*)previous_block - previous_block->size;
    freelist_block_t* next_block = address + block->size + sizeof(freelist_block_t);
    freelist_explicit_t* next_explicit = (void*)next_block + sizeof(freelist_block_t);

    enum coalesce_state state = 0;
    state |= coalesce_state_previous * (!previous_block->allocated && next_block->size);
    state |= coalesce_state_next * (!next_block->allocated && next_block->size);

    switch (state) {
        case coalesce_state_default:
            // SDEBUG("Coalesce default");
            block->allocated = false;
            block_header->allocated = false;

            if (new_explicit < allocator->first_free_block) {
                new_explicit->next = allocator->first_free_block;
                new_explicit->next->previous = new_explicit;
                allocator->first_free_block = new_explicit;
                break;
            }

            previous_explicit = allocator->first_free_block;
            while (previous_explicit) {
                if (previous_explicit->next > new_explicit) {
                    new_explicit->next = previous_explicit->next;
                    new_explicit->previous = previous_explicit;

                    if (new_explicit->next) {
                        new_explicit->next->previous = new_explicit;
                    }
                    previous_explicit->next = new_explicit;
                    break;
                }
                previous_explicit = previous_explicit->next;
            }
                allocator->first_free_block = new_explicit;
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

            new_explicit->previous = next_explicit->previous;
            new_explicit->next = next_explicit->next;

            if (new_explicit->next) {
                next_explicit->next->previous = new_explicit;
            }
            if (new_explicit->previous) {
                next_explicit->previous->next = new_explicit;
            }

            if (next_explicit == allocator->first_free_block) {
                allocator->first_free_block = new_explicit;
            }
            break;
        case coalesce_state_all:
            // SDEBUG("Coalesce all");
            previous_block = (void*)previous_block - previous_block->size - sizeof(freelist_block_t);
            previous_block->size += block->size + next_block->size + sizeof(freelist_block_t) * 4;
            block = previous_block;
            block_header = (void*)block_header + next_block->size + sizeof(freelist_block_t) * 2;
            block_header->size = block->size;

            new_explicit->next = next_explicit->next;
            previous_explicit->next = next_explicit->next;
            if (next_explicit->next) {
                next_explicit->next->previous = previous_explicit;
            }
            break;
    }
    //
    // test_explicit_freelist(allocator);
}

