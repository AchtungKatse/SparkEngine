#pragma once
#include "Spark/ecs/entity.h"
#include "Spark/ecs/ecs.h"

typedef struct entity_child {
    darray_entity_t children;
} entity_child_t;

extern ECS_COMPONENT_DECLARE(entity_child_t);
