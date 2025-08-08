#include "Spark/containers/unordered_map.h"

hashmap_type(u64, hashmap_u64);
hashmap_impl(u64, u64, hashmap_u64, hash_passthrough);

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

