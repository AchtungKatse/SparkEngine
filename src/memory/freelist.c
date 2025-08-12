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

// void* freelist_suballocate(freelist_t* allocator, u64 size);
// void test_explicit_freelist(freelist_t* allocator);

void freelist_create(u64 size, b8 aligned, freelist_t* out_allocator) {
    size = smax(size, 1024);

    const u64 memory_size = size + sizeof(freelist_block_t) * 4;
    // out_allocator->memory_size    = size + sizeof(freelist_block_t) * 4;
    out_allocator->memory         = platform_allocate(memory_size, aligned);
    out_allocator->aligned        = aligned;
    // out_allocator->next_allocator = NULL;

    // Create first block
    freelist_block_t* block = out_allocator->memory + sizeof(freelist_block_t);
    block->size             = size;
    block->allocated        = false;

    freelist_block_t* block_end = out_allocator->memory + sizeof(freelist_block_t) + size;
    block_end->size             = size;
    block_end->allocated        = false;

    freelist_explicit_t* explicit = (void*)block + sizeof(freelist_block_t);
    explicit->next     = NULL;
    explicit->previous = NULL;
    out_allocator->first_free_block  = explicit;

    // Set end of memory block
    freelist_block_t* leading_block = out_allocator->memory;
    leading_block->size = 0;
    leading_block->allocated = true;

    freelist_block_t* trailing_block = out_allocator->memory + memory_size - sizeof(freelist_block_t);
    trailing_block->size = 0;
    trailing_block->allocated = true;
    // SDEBUG("Last block addr: %p", size + sizeof(freelist_block_t));

    // test_explicit_freelist(out_allocator);
}

void freelist_destroy(freelist_t* allocator) {
    platform_free(allocator->memory, allocator->aligned);
}

// void* freelist_suballocate(freelist_t* allocator, u64 size) {
//     // Failed to allocate, check sub allocator
//     // if (allocator->next_allocator) {
//     //     // SWARN("Suballocator allocation. Size: %d", size);
//     //     return freelist_allocate(allocator->next_allocator, size);
//     // }
//
//     // Failed to allocate the data, Create a new sub-allocator
//     // SWARN("Creating sub-allocator");
//     allocator->next_allocator = platform_allocate(sizeof(freelist_t), allocator->aligned);
//     freelist_create(smax(size * 4, GENERAL_ALLOCATOR_DEFAULT_SIZE), allocator->aligned, allocator->next_allocator);
//     return freelist_allocate(allocator->next_allocator, size);
// }

void* freelist_allocate(freelist_t* allocator, u64 size) {
    // SASSERT(allocator->first_free_block, "FREELIST RAN OUT OF MEMORY.");
    // if (!allocator->first_free_block) {
    //     return freelist_suballocate(allocator, size);
    // }

    size = smax(size, 32);
    // SASSERT(size > 0, "Cannot allocate nothing.");

    // Find first block with valid size

    freelist_explicit_t* explicit = allocator->first_free_block;
    while (explicit) {
        freelist_block_t* block = (void*)allocator->first_free_block - sizeof(freelist_block_t);

        // Check if block can contain allocation
        if (size > block->size) {
            explicit = explicit->next;
            SDEBUG("Block size too small: 0x%x < 0x%x", block->size, size);
            continue;
        }

        // It can
        // Split the block into the allocation and a new block
        const u32 remaining_size = block->size - size;

        const u8 create_new_block = sizeof(freelist_block_t) < remaining_size;
        if (create_new_block) {
            freelist_block_t* new_block = ((void*)block) + sizeof(freelist_block_t) * 2 + size;
            freelist_explicit_t* new_explicit = (void*)new_block + sizeof(freelist_block_t);
            new_block->size = remaining_size - sizeof(freelist_block_t) * 2;
            new_explicit->next = explicit->next;
            new_explicit->previous = explicit->previous;

            if (explicit->next) {
                explicit->next->previous = new_explicit;
            }
            if (explicit->previous) {
                explicit->previous->next = new_explicit;
            }

            // NOTE: Following block is equivalent
            // if (explicit == allocator->first_free_block) {
            //     allocator->first_free_block = new_explicit;
            // }
            const u8 replace_first_block = explicit == allocator->first_free_block;
            allocator->first_free_block = (void*)((size_t)allocator->first_free_block * (!replace_first_block)
                + (size_t)new_explicit * replace_first_block);

            // SASSERT(new_block->size > 0, "New block size should not be zero.");
        } else {
            // size = remaining_size;
            allocator->first_free_block = explicit->next;
        }



        // Create a new allocation
        freelist_block_t* block_end = ((void*)block) + sizeof(freelist_block_t) + size;
        block_end->size      = size;
        block_end->allocated = true;

        block->size          = size;
        block->allocated     = true;
        // SASSERT(block->size > 0, "Block size should not be zero.");

        // test_explicit_freelist(allocator);
        return ((void*)block) + sizeof(freelist_block_t);
    }

    // test_explicit_freelist(allocator);
    // return freelist_suballocate(allocator, size);
    return NULL;
}

// void test_explicit_freelist(freelist_t* allocator) {
//     freelist_explicit_t* explicit = allocator->first_free_block;
//
//     // Check total available size
//     u32 total_size = sizeof(freelist_block_t) * 2;
//     freelist_block_t* _block = allocator->first_block;
//     while (_block && _block->size) {
//         total_size += _block->size + sizeof(freelist_block_t) * 2;
//         _block = (void*)_block + _block->size + sizeof(freelist_block_t) * 2;
//     }
//
//
//     SASSERT(total_size == allocator->memory_size, "Total tracked size is not the same as allocated size. Expected 0x%x, got 0x%x", allocator->memory_size, total_size);
//
//     int iterations = 0;
//     int block_count = 1;
//     while (explicit) {
//         freelist_block_t* block = (void*)explicit - sizeof(freelist_block_t);
//
//         freelist_block_t* _block = allocator->first_block;
//         while (_block != block && _block && _block->size) {
//             _block = (void*)_block + _block->size + sizeof(freelist_block_t) * 2;
//             // SDEBUG("Block: %p", _block);
//             block_count++;
//         }
//
//         SASSERT(block->size > 0,    "Failed to manage explicit freelist: Targeting enclosing blocks.");
//         // SASSERT(!block->allocated,  "Failed to manage explicit freelist: Expected free block.");
//         SASSERT(iterations < 10000, "Failed to manage explicit freelist: Recursive freelist detected.");
//         SASSERT(_block == block,    "Failed to manage explicit freelist: Target block is not in freelist. Total block count: %d", block_count);
//         explicit = explicit->next;
//         iterations++;
//     }
// }

void freelist_free(freelist_t* allocator, void* address) {
    // while (address < allocator->memory || address > allocator->memory + allocator->memory_size) {
    //     allocator = allocator->next_allocator;
    // }
    // get allocation
    freelist_block_t* block = address - sizeof(freelist_block_t);
    freelist_block_t* block_header = address + block->size;

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
    if (!previous_block->allocated) {
        state += coalesce_state_previous;
    }
    if (!next_block->allocated) {
        state += coalesce_state_next;
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

            if (previous_explicit->previous) {
                previous_explicit->previous->next = previous_explicit->next;
            } else {
                allocator->first_free_block = NULL;
            }
            if (previous_explicit->next) {
                previous_explicit->next->previous = previous_explicit->previous;
            }
            new_explicit = previous_explicit;
            break;
        case coalesce_state_next:
            // SDEBUG("Coalesce next");
            block->allocated = false;
            block->size += next_block->size + sizeof(freelist_block_t) * 2;
            block_header = (void*)block_header + next_block->size + sizeof(freelist_block_t) * 2;
            block_header->size = block->size;

            if (next_explicit->previous) {
                next_explicit->previous->next = next_explicit->next;
            } else {
                allocator->first_free_block = NULL;
            }

            if (next_explicit->next) {
                next_explicit->next->previous = next_explicit->previous;
            }
            break;
        case coalesce_state_all:
            // SDEBUG("Coalesce all");
            previous_block = (void*)previous_block - previous_block->size - sizeof(freelist_block_t);
            previous_block->size += block->size + next_block->size + sizeof(freelist_block_t) * 4;
            block = previous_block;
            block_header = (void*)previous_block + previous_block->size + sizeof(freelist_block_t);
            block_header->size = block->size;

            if (next_explicit->previous) {
                next_explicit->previous->next = next_explicit->next;
            }
            if (next_explicit->next) {
                next_explicit->next->previous = next_explicit->previous;
            }

            if (previous_explicit->previous) {
                previous_explicit->previous->next = previous_explicit->next;
            }
            if (previous_explicit->next) {
                previous_explicit->next->previous = previous_explicit->previous;
            }

            if (allocator->first_free_block == previous_explicit) {
                allocator->first_free_block = NULL;
            }
            if (allocator->first_free_block == next_explicit) {
                allocator->first_free_block = NULL;
            }

            new_explicit = previous_explicit;
            break;
    }

    // block->allocated = false;
    // block_header->allocated = false;
    // if (!previous_block->allocated) {
    //     freelist_explicit_t* previous_explicit = (void*)previous_block - previous_block->size;
    //     previous_block = (void*)previous_block - previous_block->size - sizeof(freelist_block_t);
    //     previous_block->size += block->size + sizeof(freelist_block_t) * 2;
    //     block = previous_block;
    //     block_header->size = block->size;
    //     block_header->allocated = false;
    //
    //     if (previous_explicit->previous) {
    //         previous_explicit->previous->next = previous_explicit->next;
    //     } else {
    //         allocator->first_free_block = NULL;
    //     }
    //     if (previous_explicit->next) {
    //         previous_explicit->next->previous = previous_explicit->previous;
    //     }
    //     new_explicit = previous_explicit;
    // }
    // if (!next_block->allocated) {
    //     freelist_explicit_t* next_explicit = (void*)next_block + sizeof(freelist_block_t);
    //     block->allocated = false;
    //     block->size += next_block->size + sizeof(freelist_block_t) * 2;
    //     block_header = (void*)block_header + next_block->size + sizeof(freelist_block_t) * 2;
    //     block_header->size = block->size;
    //
    //     if (next_explicit->previous) {
    //         next_explicit->previous->next = next_explicit->next;
    //     } else {
    //         allocator->first_free_block = NULL;
    //     }
    //
    //     if (next_explicit->next) {
    //         next_explicit->next->previous = next_explicit->previous;
    //     }
    // }

    new_explicit->next                    = allocator->first_free_block;
    new_explicit->previous                = NULL;
    if (allocator->first_free_block) {
        allocator->first_free_block->previous = new_explicit;
    }
    allocator->first_free_block           = new_explicit;

    // test_explicit_freelist(allocator);
    // SASSERT((void*)allocator->first_free_block >= allocator->memory && (void*)allocator->first_free_block < allocator->memory + allocator->memory_size, "First free block is oob");
}

// void freelist_print_debug_alloc_internal(freelist_t* allocator, u32 suballocator_index) {
//     u32 total_size = sizeof(freelist_block_t) * 2;
//     u32 reserved_size = sizeof(freelist_block_t) * 2;
//     u32 free_size = 0;
//     u32 allocated_size = 0;
//     freelist_block_t* block = allocator->memory + sizeof(freelist_block_t);
//     while ((void*)block >= allocator->memory && (void*)block < allocator->memory + allocator->memory_size) {
//         total_size += block->size + sizeof(freelist_block_t) * 2;
//
//         reserved_size += sizeof(freelist_block_t) * 2;
//         if (block->allocated) {
//             allocated_size += block->size;
//         } else {
//             free_size += block->size;
//         }
//
//         block = (void*)block + block->size + sizeof(freelist_block_t) * 2;
//     }
//
//     SDEBUG("Index %d, Total: 0x%x, Reserved: 0x%x, Allocated: 0x%x, Free: 0x%x", suballocator_index, total_size, reserved_size, allocated_size, free_size);
// }
//
// void freelist_print_debug_allocations(freelist_t *allocator) {
//
//     freelist_print_debug_alloc_internal(allocator, 0);
//     freelist_t* suballocator = allocator->next_allocator;
//     u32 suballocator_index = 1;
//     while (suballocator) {
//         freelist_print_debug_alloc_internal(suballocator, suballocator_index);
//         suballocator = suballocator->next_allocator;
//     }
// }
