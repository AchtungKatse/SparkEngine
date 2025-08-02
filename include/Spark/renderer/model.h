#pragma once

#include "Spark/renderer/mesh.h"
#include "Spark/types/aabb.h"
typedef struct model {
    mesh_t mesh;
    aabb_t bounds;

    vec3 translation;
    vec3 scale;
    quat rotation;

    u16 child_count;
    u16 material_index;
    struct model* children;
    struct model* next_child;
} model_t;

typedef struct model_config {
    const char* path;
} model_config_t;
