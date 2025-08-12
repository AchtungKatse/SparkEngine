#include "Spark/core/logging.h"
#include "Spark/memory/freelist.h"

void freelist_test() {
    // freelist_t allocator;
    // const u32 memory_size = 16 * KB;
    // freelist_create(memory_size, false, &allocator);
    // freelist_block_t block = *allocator.first_block;
    //
    // // Allocate a value well under the limit
    // const u32 int_count = 64;
    // // Simple allocation and free test
    // {
    //     int* ints = freelist_allocate(&allocator, sizeof(int) * int_count);
    //
    //     for (u32 i = 0; i < int_count; i++) {
    //         ints[i] = i;
    //     }
    //
    //     for (u32 i = 0; i < int_count; i++) {
    //         SASSERT(ints[i] == i, "This should not be possible");
    //     }
    //
    //     freelist_free(&allocator, ints);
    //     // New block size should be equal to the old block size due to defragmentation
    //     if (block.size != allocator.first_block->size) {
    //         SERROR("Failed to defragment general allocator. Expected size of 0x%x, got 0x%x", block.size, allocator.first_block->size);
    //     } else {
    //         SINFO("Freelist basic alloc / dealloc success");
    //     }
    // }
    //
    //
    // // Gap defragmentation test
    // // This creates 4 allocations and frees them to create gaps
    // // This checks if the allocator is capable of automatically defragmenting
    // { 
    //     const u32 int_count = 8;
    //     int* ints[int_count];
    //     for (u32 i = 0; i < int_count; i++) {
    //         ints[i] = freelist_allocate(&allocator, sizeof(int) * int_count);
    //     }
    //
    //     freelist_free(&allocator, ints[0]);
    //     freelist_free(&allocator, ints[3]);
    //     freelist_free(&allocator, ints[2]);
    //     freelist_free(&allocator, ints[1]);
    //     freelist_free(&allocator, ints[7]);
    //     freelist_free(&allocator, ints[4]);
    //     freelist_free(&allocator, ints[5]);
    //     freelist_free(&allocator, ints[6]);
    //
    //     if (block.size == allocator.first_block->size) {
    //         SINFO("General allocator passed gap defragmentation test");
    //     } else {
    //         SERROR("Failed to defragment general allocator. Expected size of 0x%x, got 0x%x. First Block: %p", block.size, allocator.first_block->size, allocator.first_block);
    //     }
    // }
    //
    // // Allocate something over the allocator size
    // {
    //     const u32 max_int_count = memory_size / sizeof(int);
    //     int* ints = freelist_allocate(&allocator, memory_size * 2);
    //
    //     for (u32 i = 0; i < max_int_count; i++) {
    //         ints[i] = i;
    //     }
    //
    //     for (u32 i = 0; i < max_int_count; i++) {
    //         SASSERT(ints[i] == i, "This should not be possible");
    //     }
    //     freelist_free(&allocator, ints);
    //     if (block.size != allocator.first_block->size) {
    //         SERROR("Failed to defragment general allocator. Expected size of 0x%x, got 0x%x", block.size, allocator.first_block->size);
    //     } else {
    //         SINFO("Freelist chunk overflow test success.");
    //     }
    // }
    //
    //
    // // Random alloc dealloc
    // {
    //     int* rand_ints[int_count];
    //     for (u32 i = 0; i < int_count; i++) {
    //         rand_ints[i] = freelist_allocate(&allocator, sizeof(int));
    //     }
    //
    //     for (u32 i = 0; i < 10000; i++) {
    //         u32 index = random() % int_count;
    //         if (rand_ints[index]) {
    //             freelist_free(&allocator, rand_ints[index]);
    //             rand_ints[index] = NULL;
    //         }
    //     }
    //
    //     for (u32 i = 0; i < int_count; i++) {
    //         if (rand_ints[i]) {
    //             freelist_free(&allocator, rand_ints[i]);
    //         }
    //     }
    //
    //     if (block.size != allocator.first_block->size) {
    //         SERROR("Failed to defragment general allocator. Expected size of 0x%x, got 0x%x", block.size, allocator.first_block->size);
    //     } else {
    //         SINFO("Freelist random deallocate success");
    //     }
    // }
    //
    // // Overflow test
    // for (u32 i = 1; i <= 16; i++) {
    //     u32 alloc_size = 1024;
    //     const u32 max_int_count = memory_size / (alloc_size) * i;
    //     int* ints[max_int_count];
    //
    //     for (u32 i = 0; i < max_int_count; i++) {
    //         ints[i] = freelist_allocate(&allocator, alloc_size);
    //     }
    //
    //     for (u32 i = 0; i < max_int_count * 50; i++) {
    //         u32 index = random() % max_int_count;
    //         if (ints[index]) {
    //             freelist_free(&allocator, ints[index]);
    //             ints[index] = NULL;
    //         }
    //     }
    //     for (u32 i = 0; i < max_int_count; i++) {
    //         if (ints[i]) {
    //             freelist_free(&allocator, ints[i]);
    //         }
    //     }
    //
    //     if (block.size != allocator.first_block->size) {
    //         SERROR("Failed to defragment general allocator. Expected size of 0x%x, got 0x%x", block.size, allocator.first_block->size);
    //     } else {
    //         SINFO("Freelist overflow test success");
    //     }
    // }
}
