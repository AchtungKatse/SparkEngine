#pragma once

#include "Spark/ecs/ecs.h"
#include "Spark/ecs/entity.h"

typedef struct mesh {
    u32 internal_index;
} mesh_t;

extern ECS_COMPONENT_DECLARE(mesh_t);
