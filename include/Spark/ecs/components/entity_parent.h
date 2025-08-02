#pragma once
#include "Spark/ecs/entity.h"
#include "Spark/ecs/ecs.h"

typedef struct entity_parent {
    entity_t parent;
} entity_parent_t;

extern ECS_COMPONENT_DECLARE(entity_parent_t);
