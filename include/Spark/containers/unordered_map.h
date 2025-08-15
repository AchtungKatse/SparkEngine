#pragma once

#include "Spark/core/logging.h"
#include "Spark/core/smemory.h"
#include "Spark/utils/hashing.h"
#define HASHMAP_MAX_LINKED_LIST_LENGTH 3

#define hashmap_type(type, name)                                                                                                          \
    typedef struct name##_hash_pair {                                                                                                           \
        u64 hash;                                                                                                                               \
        type value;                                                                                                                             \
        struct name##_hash_pair* next;                                                                                                          \
    } name##_hash_pair_t;                                                                                                                       \
    typedef struct name {                                                                                                                       \
        u32 capacity;                                                                                                                           \
        u32 count;                                                                                                                              \
        name##_hash_pair_t** table;                                                                                                             \
        name##_hash_pair_t* linked_lists;                                                                                                       \
    } name ##_t

#define hashmap_header(key_type, value_type, name)                                                                                                           \
    hashmap_type(value_type, name);                                                                                                                          \
    void name ##_create(u32 capacity, struct name* out_map);                                                                                                 \
    void name ##_destroy(struct name* map);                                                                                                                  \
    value_type* name ##_insert(struct name* map, key_type key, value_type value);                                                                            \
    void name##_resize(struct name* map, u32 size);                                                                                                          \
    b8 name ##_try_get(struct name* map, key_type key, value_type* out_value);                                                                               \
    value_type* name ##_get(struct name* map, key_type key);                                                                                                 \
    b8 name ##_contains(struct name* map, key_type key);

#define hashmap_impl(key_type, value_type, name, hash_function)                                                                                              \
    void name##_resize(struct name* map, u32 size) {                                                                                                         \
        name##_hash_pair_t** tmp_table = sallocate(size * sizeof(name##_hash_pair_t*), MEMORY_TAG_ARRAY);                                                    \
        name##_hash_pair_t* tmp_linked_lists = sallocate(size * sizeof(name##_hash_pair_t) * HASHMAP_MAX_LINKED_LIST_LENGTH, MEMORY_TAG_ARRAY);              \
        for (u32 i = 0; i < size * HASHMAP_MAX_LINKED_LIST_LENGTH; i++) { tmp_linked_lists[i].hash = INVALID_ID_U64; }                                       \
        for (u32 i = 0, count = 0; i < map->capacity; i++) {                                                                                                 \
            if (map->linked_lists[i].hash == INVALID_ID_U64) {                                                                                               \
                continue;                                                                                                                                    \
            }                                                                                                                                                \
            u32 new_index = map->linked_lists[i].hash % size;                                                                                                \
            name##_hash_pair_t* pair = tmp_table[new_index];                                                                                                 \
            name##_hash_pair_t** pair_pointer = &tmp_table[new_index];                                                                                       \
            while (pair != NULL) {                                                                                                                           \
                pair_pointer = &pair->next;                                                                                                                  \
                pair = pair->next;                                                                                                                           \
            }                                                                                                                                                \
            pair = &tmp_linked_lists[count++];                                                                                                               \
            pair->hash = map->linked_lists[i].hash;                                                                                                          \
            pair->value = map->linked_lists[i].value;                                                                                                        \
            *pair_pointer = pair;                                                                                                                            \
        }                                                                                                                                                    \
        sfree(map->table, map->capacity * sizeof(name##_hash_pair_t*), MEMORY_TAG_ARRAY);                                                                    \
        map->table = NULL;                                                                                                                                   \
        sfree(map->linked_lists, map->capacity * sizeof(name##_hash_pair_t) * HASHMAP_MAX_LINKED_LIST_LENGTH, MEMORY_TAG_ARRAY);                             \
        map->table = tmp_table;                                                                                                                              \
        map->linked_lists = tmp_linked_lists;                                                                                                                \
        map->capacity = size;                                                                                                                                \
    }                                                                                                                                                        \
    void name##_create(u32 capacity, struct name* out_map) {                                                                                                 \
        out_map->capacity = capacity;                                                                                                                        \
        out_map->count = 0;                                                                                                                                  \
        out_map->table = sallocate(capacity * sizeof(name##_hash_pair_t*), MEMORY_TAG_ARRAY);                                                                \
        out_map->linked_lists = sallocate(capacity * sizeof(name##_hash_pair_t) * HASHMAP_MAX_LINKED_LIST_LENGTH, MEMORY_TAG_ARRAY);                         \
        for (u32 i = 0; i < capacity * HASHMAP_MAX_LINKED_LIST_LENGTH; i++) { out_map->linked_lists[i].hash = INVALID_ID_U64; }                              \
    }                                                                                                                                                        \
    void name##_destroy(struct name* map) {                                                                                                                  \
        if (map->linked_lists) {                                                                                                                             \
            sfree(map->linked_lists, map->capacity * sizeof(name##_hash_pair_t) * HASHMAP_MAX_LINKED_LIST_LENGTH, MEMORY_TAG_ARRAY);                         \
            map->linked_lists = NULL;                                                                                                                        \
        }                                                                                                                                                    \
        if (map->table) {                                                                                                                                    \
            sfree(map->table, map->capacity * sizeof(name##_hash_pair_t*), MEMORY_TAG_ARRAY);                                                                \
            map->table = NULL;                                                                                                                               \
        }                                                                                                                                                    \
    }                                                                                                                                                        \
    value_type* name##_insert(struct name* map, key_type key, value_type value) {                                                                            \
        u64 hash = hash_function(key);                                                                                                                       \
        u32 index = hash % map->capacity;                                                                                                                    \
        name##_hash_pair_t* pair = map->table[index];                                                                                                        \
        name##_hash_pair_t** pair_pointer = &map->table[index];                                                                                              \
        for (u32 i = 0; i < HASHMAP_MAX_LINKED_LIST_LENGTH; i++) {                                                                                           \
            if (pair != NULL) {                                                                                                                              \
                pair_pointer = &pair->next;                                                                                                                  \
                pair = pair->next;                                                                                                                           \
                continue;                                                                                                                                    \
            }                                                                                                                                                \
            pair = &map->linked_lists[map->count++];                                                                                                         \
            *pair_pointer = pair;                                                                                                                            \
            pair->value = value;                                                                                                                             \
            pair->hash = hash;                                                                                                                               \
            return &pair->value;                                                                                                                             \
        }                                                                                                                                                    \
        SINFO("Resizing hash map to prevent collision");                                                                                                     \
        name##_resize(map, map->capacity * 2);                                                                                                               \
        return name##_insert(map, key, value);                                                                                                               \
    }                                                                                                                                                        \
    b8 name ##_try_get(struct name* map, key_type key, value_type* out_value) {                                                                              \
        u64 hash = hash_function(key);                                                                                                                       \
        u64 index = hash % map->capacity;                                                                                                                    \
        name##_hash_pair_t* pair = map->table[index];                                                                                                        \
        while (pair != NULL) {                                                                                                                               \
            if (pair->hash == hash) {                                                                                                                        \
                *out_value = pair->value;                                                                                                                    \
                return true;                                                                                                                                 \
            }                                                                                                                                                \
            pair = pair->next;                                                                                                                               \
        }                                                                                                                                                    \
        return false;                                                                                                                                        \
    }                                                                                                                                                        \
    value_type* name##_get(struct name* map, key_type key) {                                                                                                 \
        u64 hash = hash_function(key);                                                                                                                       \
        u64 index = hash % map->capacity;                                                                                                                    \
        name##_hash_pair_t* pair = map->table[index];                                                                                                        \
        while (pair != NULL) {                                                                                                                               \
            if (pair->hash == hash) {                                                                                                                        \
                return &pair->value;                                                                                                                         \
            }                                                                                                                                                \
            pair = pair->next;                                                                                                                               \
        }                                                                                                                                                    \
        return NULL;                                                                                                                                         \
    }                                                                                                                                                        \
    b8 name##_contains(struct name* map, key_type key) {                                                                                                     \
        u64 hash = hash_function(key);                                                                                                                       \
        u64 index = hash % map->capacity;                                                                                                                    \
        name##_hash_pair_t* pair = map->table[index];                                                                                                        \
        while (pair != NULL) {                                                                                                                               \
            if (pair->hash == hash) {                                                                                                                        \
                return true;                                                                                                                                 \
            }                                                                                                                                                \
            pair = pair->next;                                                                                                                               \
        }                                                                                                                                                    \
        return false;                                                                                                                                        \
    }
