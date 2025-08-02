#pragma once

#include "Spark/containers/darray.h"

typedef struct vulkan_mesh {
    u32 index_start;
    u32 index_count;
    u32 vertex_start;
    u32 vertex_count;
    u32 vertex_stride;
} vulkan_mesh_t;

darray_header(vulkan_mesh_t, vulkan_mesh);

