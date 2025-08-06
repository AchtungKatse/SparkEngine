#include "Spark/defines.h"
#include "Spark/core/logging.h"

#include "Spark/containers/unordered_map.h"
#include "Spark/memory/gerneal_allocator.h"
#include "Spark/utils/hashing.h"
#include <stdlib.h>

hashmap_type(u64, hashmap_u64);
hashmap_impl(u64, u64, hashmap_u64, hash_passthrough);
b8 hashmap_test();
b8 general_allocator_test();

s32 main(s32 argc, char** argv) {
    // if (hashmap_test()) {
    //     SINFO("Hashmap test success");
    // } else {
    //     SERROR("Failed hashmap tests");
    // }

    if (general_allocator_test()) {
        SINFO("General Allocator test success");
    } else {
        SERROR("Failed general allocator tests");
    }
}

b8 hashmap_test() {
    const u32 value_count = 100000;
    u64 rand_values[value_count];
    u64 rand_keys[value_count];
    for (u32 i = 0; i < value_count; i++) {
        rand_values[i] = random();
        rand_keys[i] = random();
    }

    hashmap_u64_t map;
    hashmap_u64_create(100, &map);

    for (u32 i = 0; i < value_count; i++) {
        hashmap_u64_insert(&map, rand_keys[i], rand_values[i]);
    }

    for (u32 i = 0; i < value_count; i++) {
        if (*hashmap_u64_get(&map, rand_keys[i]) != rand_values[i]) {
            SERROR("Hashmap implementation failed tests. Did not receive same value key pair after insertions. Key: 0x%x, Value: 0x%x, Expected Value: 0x%x", 
                    rand_keys[i], 
                    *hashmap_u64_get(&map, rand_keys[i]), 
                    rand_values[i]);
            return false;
        }
    }

    return true;
}

b8 general_allocator_test() {
    general_allocator_t allocator;
    const u32 memory_size = 10 * MB;
    general_allocator_create(memory_size, false, &allocator);
    general_allocator_block_t block = *allocator.first_block;

    // Allocate a value well under the limit
    const u32 int_count = 64;
    int* ints = general_allocator_allocate(&allocator, sizeof(int) * int_count, MEMORY_TAG_ARRAY);

    for (u32 i = 0; i < int_count; i++) {
        ints[i] = i;
    }

    for (u32 i = 0; i < int_count; i++) {
        SASSERT(ints[i] == i, "This should not be possible");
    }

    general_allocator_free(&allocator, ints);

    // New block size should be equal to the old block size due to defragmentation
    SASSERT(block.size == allocator.first_block->size, "Failed to defragment general allocator. Expected size of 0x%x, got 0x%x", block.size, allocator.first_block->size);

    // Allocate something over the allocator size
    const u32 max_int_count = memory_size / sizeof(int);
    ints = general_allocator_allocate(&allocator, max_int_count * 2, MEMORY_TAG_ARRAY);

    for (u32 i = 0; i < max_int_count; i++) {
        ints[i] = i;
    }

    for (u32 i = 0; i < max_int_count; i++) {
        SASSERT(ints[i] == i, "This should not be possible");
    }

    return true;
}

