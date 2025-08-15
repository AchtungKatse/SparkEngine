#include "Spark/memory/dynamic_allocator.h"
#include "Spark/memory/freelist.h"
#include "Spark/platform/platform.h"

void dynamic_allocator_create(u64 memory_size, dynamic_allocator_t* out_allocator) {
    void* memory = platform_allocate(memory_size, true);
    freelist_create(memory, memory_size, &out_allocator->freelist);
    out_allocator->memory_size = memory_size;
    out_allocator->next_allocator = NULL;
}
void dynamic_allocator_destroy(dynamic_allocator_t* allocator) {
    freelist_destroy(&allocator->freelist);
    if (allocator->next_allocator) {
        dynamic_allocator_destroy(allocator->next_allocator);
    }
}

void* dynamic_allocator_allocate(dynamic_allocator_t* allocator, u64 size) {
    void* data = freelist_allocate(&allocator->freelist, size);
    if (data) {
        return data;
    }

    // Failed to allocate data, try next allocator
    if (allocator->next_allocator) {
        return dynamic_allocator_allocate(allocator->next_allocator, size);
    }

    // No existing allocator, try creating one.
    allocator->next_allocator = platform_allocate(sizeof(dynamic_allocator_t), true);
    dynamic_allocator_create(allocator->memory_size, allocator->next_allocator);
    return dynamic_allocator_allocate(allocator->next_allocator, size);
}

void dynamic_allocator_free(dynamic_allocator_t* allocator, void* data) {
    while (data < allocator->freelist.memory || data > allocator->freelist.memory + allocator->memory_size) {
        allocator = allocator->next_allocator;
    }

    freelist_free(&allocator->freelist, data);
}
