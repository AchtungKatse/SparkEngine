#pragma once

#include "Spark/core/smemory.h"
#include "Spark/defines.h"

#define DARRAY_RESIZE_FACTOR 2

#define darray_type(type, name)                                                                                                                   \
    typedef struct darray_ ##name {                                                                                                               \
        type * data;                                                                                                                              \
        u32 capacity;                                                                                                                             \
        u32 count;                                                                                                                                \
    } darray_ ##name ##_t

#define darray_header(type, name)                                                                                                                 \
    darray_type(type, name);                                                                                                                      \
    void darray_ ##name ##_create(u32 initial_size, struct darray_ ##name* out_array);                                                            \
    void darray_ ##name ##_destroy(struct darray_##name* array);                                                                                  \
    type* darray_ ##name ##_push(struct darray_##name* array, const type value);                                                                         \
    void darray_ ##name ##_push_range(struct darray_##name* array, u32 count, const type* values);                                                      \
    type darray_ ##name ##_pop(struct darray_##name* array,u32 index);                                                                            \
    void darray_ ##name ##_pop_range(struct darray_##name* array,u32 count, u32 start_index);                                                     \
    void darray_##name##_reserve(struct darray_##name* array,u32 size);                                                                           \
    void darray_##name##_clear(struct darray_##name* array);

#define darray_impl(type, name)                                                                                                                         \
    void darray_ ##name ##_create(u32 initial_size, struct darray_ ##name* out_array) {                                                                 \
        out_array->capacity = initial_size;                                                                                                             \
        out_array->count = 0;                                                                                                                           \
        SASSERT(sizeof(type) != 0, "Cannot create darray of type '" #type "' with size of 0.");                                                         \
        out_array->data = sallocate(sizeof(type) * initial_size, MEMORY_TAG_DARRAY);                                                                    \
    }                                                                                                                                                   \
    void darray_ ##name ##_destroy(struct darray_##name* array) {                                                                                       \
        SASSERT(array->data, "Cannot operate on null darray");                                                                                          \
        if (array->data) {                                                                                                                              \
            sfree(array->data, sizeof(type) * array->capacity, MEMORY_TAG_DARRAY);                                                                      \
            array->data = NULL;                                                                                                                         \
        }                                                                                                                                               \
    }                                                                                                                                                   \
    void darray_##name##_reserve(struct darray_##name* array,u32 size);                                                                                 \
    type* darray_ ##name ##_push(struct darray_##name* array, const type value) {                                                                             \
        SASSERT(array, "Cannot operate on null darray");                                                                                          \
        SASSERT(array->data, "Cannot operate on uninitialized");                                                                                          \
        if (array->count >= array->capacity) {                                                                                                          \
            darray_##name##_reserve(array, array->capacity * 2);                                                                                        \
        }                                                                                                                                               \
        u32 array_index = array->count;                                                                                                                 \
        array->count++;                                                                                                                                 \
        scopy_memory(&array->data[array_index], &value, sizeof(type));                                                                                                               \
        return &array->data[array_index];                                                                                                               \
    }                                                                                                                                                   \
    void darray_ ##name ##_push_range(struct darray_##name* array, u32 count, const type* values) {                                                           \
        SASSERT(array->data, "Cannot operate on null darray");                                                                                          \
        scopy_memory(array->data + array->count, values, count * sizeof(type));                                                                         \
        array->count += count;                                                                                                                          \
    }                                                                                                                                                   \
    type darray_ ##name ##_pop(struct darray_##name* array, u32 index) {                                                                                \
        SASSERT(array->data, "Cannot operate on null darray");                                                                                          \
        SASSERT(index >= 0 && index < array->count, "Darray tring to pop out of bounds index: %d. Count: %d", index, array->count);                     \
        type value = array->data[index];                                                                                                                \
        if (index < array->count) {                                                                                                                     \
            for (u32 i = index; i < array->count - 1; i++) {                                                                                            \
                array->data[i] = array->data[i - 1];                                                                                                    \
            }                                                                                                                                           \
        }                                                                                                                                               \
        array->count--;                                                                                                                                 \
        return value;                                                                                                                                   \
    }                                                                                                                                                   \
    void darray_ ##name ##_pop_range(struct darray_##name* array, u32 count, u32 start_index) {                                                         \
        SASSERT(array->data, "Cannot operate on null darray");                                                                                          \
        SASSERT(start_index >= 0 && start_index + count < array->count,                                                                                 \
                "Unable to pop range of values in darray. Start Index: %d, Pop Count: %d, Array Count: %d", start_index, count, array->count);          \
        if (start_index + count < array->count) {                                                                                                       \
            scopy_memory(&array->data[start_index], &array->data[start_index + count], sizeof(type) * array->count - (start_index + count));            \
        }                                                                                                                                               \
        array->count -= count;                                                                                                                          \
    }                                                                                                                                                   \
    void darray_##name##_reserve(struct darray_##name* array, u32 size) {                                                                               \
        SASSERT(sizeof(type) != 0, "Cannot create darray of type '" #type "' with size of 0.");                                                         \
        if (array->capacity >= size) {                                                                                                                  \
            return;                                                                                                                                     \
        }                                                                                                                                               \
        type* temp = sallocate(sizeof(type) * size, MEMORY_TAG_DARRAY);                                                                                 \
        scopy_memory(temp, array->data, array->count * sizeof(type));                                                                                   \
        sfree(array->data, sizeof(type) * array->capacity, MEMORY_TAG_DARRAY);                                                                          \
        array->data = temp;                                                                                                                             \
        array->capacity = size;                                                                                                                         \
    }                                                                                                                                                   \
    void darray_##name##_clear(struct darray_##name* array) {                                                                                           \
        SASSERT(array->data, "Cannot operate on null darray");                                                                                          \
        array->count = 0;                                                                                                                               \
    }

