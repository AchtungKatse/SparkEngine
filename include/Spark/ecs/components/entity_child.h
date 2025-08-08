#pragma once
#include "Spark/ecs/entity.h"
#include "Spark/ecs/ecs.h"

typedef struct entity_child {
    entity_t child;
    entity_t next_sibling;
    entity_t previous_sibling;
} entity_child_t;

extern ECS_COMPONENT_DECLARE(entity_child_t);
