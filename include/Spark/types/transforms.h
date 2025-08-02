#pragma once

#include "Spark/ecs/ecs.h"
#include "Spark/math/mat4.h"
#include "Spark/math/math_types.h"

typedef struct dirty_transform {
    b8 dirty;
} dirty_transform_t;

typedef struct translation {
    vec3 value;
} translation_t;

typedef struct scale {
    vec3 value;
} scale_t;

typedef struct rotation {
    quat value;
} rotation_t;

typedef struct local_to_world {
    mat4 value;
} local_to_world_t;

typedef struct translation_2d {
    vec2 value;
} translation_2d_t;

typedef struct scale_2d {
    vec2 value;
} scale_2d_t;

// // TODO: Swap to mat3
// typedef struct local_to_world_2d {
//     mat4 value;
// } local_to_world_2d_t;

extern ECS_COMPONENT_DECLARE(dirty_transform_t);
extern ECS_COMPONENT_DECLARE(translation_t);
extern ECS_COMPONENT_DECLARE(rotation_t);
extern ECS_COMPONENT_DECLARE(scale_t);
extern ECS_COMPONENT_DECLARE(local_to_world_t);

extern ECS_COMPONENT_DECLARE(translation_2d_t);
extern ECS_COMPONENT_DECLARE(scale_2d_t);
// extern ECS_COMPONENT_DECLARE(local_to_world_2d_t);
