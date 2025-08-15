#pragma once

#include "Spark/core/smemory.h"
#include "Spark/defines.h"
#define set_type(type, name)                                                                 \
    typedef struct pvt_##name##_container { \
        type value; \
        u32 index; \
    } pvt_##name##_container_t; \
    typedef struct name {                                                                    \
        u32 count;                                                                           \
        u32 capacity;                                                                        \
        pvt_##name##_container_t* data;                                                                          \
    } name ##_t;

#define set_header(type, name)                                                               \
    set_type   (type, name);                                                                 \
    void name##_create   (u32 capacity, name##_t* out_set);                                  \
    void name##_destroy  (name##_t* set);                                                    \
    void name##_insert   (name##_t* set, type value);                                        \
    u32  name##_get_index(name##_t* set, type value);                                        \
    b8   name##_contains (name##_t* set, type value);

#define set_impl(type, name)                                                                 \
    void name##_resize(name##_t* set, u32 capacity) {                                        \
        pvt_##name##_container_t* temp = sallocate(sizeof(pvt_##name##_container_t) * capacity, MEMORY_TAG_ARRAY);                   \
        sset_memory(temp, 0xFFFFFFFF, capacity * sizeof(pvt_##name##_container_t)); \
        for (u32 i = 0; i < set->capacity; i++) { \
            u32 value = set->data[i].value; \
            u32 index = value % capacity; \
            if (value == INVALID_ID) { \
                continue; \
            } \
            if (temp[index].value != INVALID_ID) { \
                sfree(temp, sizeof(pvt_##name##_container_t) * capacity, MEMORY_TAG_ARRAY); \
                SDEBUG("Failed to resize set: Value: %d, invalid_id: %d, Index: %d, New capacity %d", value, value == INVALID_ID, index, capacity); \
                name##_resize(set, capacity * 2); \
                return; \
            } \
            temp[index].value = value; \
            temp[index].index = set->data[i].index; \
        } \
        sfree(set->data, sizeof(pvt_##name##_container_t) * set->capacity, MEMORY_TAG_ARRAY);                    \
        set->data = temp;                                                                    \
        set->capacity = capacity;                                                            \
    }                                                                                        \
    void name##_create(u32 capacity, name##_t* out_set) {                                    \
        out_set->data = sallocate(sizeof(pvt_##name##_container_t) * capacity, MEMORY_TAG_ARRAY);                \
        out_set->capacity = capacity;                                                        \
        out_set->count = 0; \
        for (u32 i = 0; i < capacity; i++) { \
            out_set->data[i].value = INVALID_ID; \
            out_set->data[i].index = INVALID_ID; \
        } \
    }                                                                                        \
    void name##_destroy(name##_t* set) {                                                     \
        sfree(set->data, sizeof(pvt_##name##_container_t) * set->capacity, MEMORY_TAG_ARRAY);                    \
        set->data = NULL;                                                                    \
    }                                                                                        \
    void name##_insert(name##_t* set, type value) {                                          \
        u32 index = value % set->capacity; \
        if (set->data[index].value != INVALID_ID || set->count >= set->capacity) { \
            name##_resize(set, set->capacity * 2); \
            name##_insert(set, value); \
            return; \
        } \
        SASSERT(set->data[index].value == INVALID_ID, "Cannot insert value %d into set, index already in use (value: %d)", value, set->data[value % set->capacity]); \
        set->data[index].value = value;                                                     \
        set->data[index].index = set->count;                                                     \
        set->count += 1; \
    }                                                                                        \
    u32 name##_get_index(name##_t* set, type value) {                                        \
        u32 index = value % set->capacity; \
        if (set->data[index].value == value) { \
            return set->data[index].index; \
        } \
        return INVALID_ID; \
    }                                                                                        \
    b8 name##_contains(name##_t* set, type value) {                                          \
        return set->data[value % set->capacity].value == value; \
    }
