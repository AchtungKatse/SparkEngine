#pragma once

#include "Spark/defines.h"

#define KB 1024
#define MB (KB * 1024)

typedef enum {
    MEMORY_TAG_UNDEFINED,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ECS,
    MEMORY_TAG_SYSTEM,
    MEMORY_TAG_STRING,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL,
    MEMORY_TAG_SHADER,
    MEMORY_TAG_MESH,
    MEMORY_TAG_GAME,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_ALLOCATOR,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_THREAD,

    MEMORY_TAG_MAX,
} memory_tag_t;

void initialize_memory();
void shutdown_memory();

SAPI void*  pvt_sallocate(u64 size, memory_tag_t tag);
SAPI void   pvt_spark_free(const void* block, u64 size, memory_tag_t tag);

SAPI void* szero_memory(void* block, u64 size);
SAPI void* sset_memory(void* block, s32 value, u64 size);
SAPI void* scopy_memory(void* dest, const void* src, u64 size);

SAPI const char* get_memory_usage_string();
SAPI u64 get_memory_alloc_count();

#if SPARK_DEBUG

void* create_tracked_allocation(u64 size, memory_tag_t tag, const char* file, u32 line);
void  free_tracked_allocation(const void* block, u32 size, memory_tag_t tag);

#define sallocate(size, tag)    create_tracked_allocation(size, tag, __FILE__, __LINE__)
#define sfree(block, size, tag) free_tracked_allocation((void*)block, size, tag)

#else

#define sfree(block, size, tag)     pvt_spark_free(block, size, tag);
#define sallocate(size, tag)        pvt_sallocate(size, tag);

#endif

