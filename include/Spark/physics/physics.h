#pragma once

#include "Spark/ecs/ecs.h"
typedef enum physics_broadphase_layer {
    PHYSICS_BROADPHASE_LAYER_NON_MOVING = 0,
    PHYSICS_BROADPHASE_LAYER_MOVING = 1,
    PHYSICS_BROADPHASE_LAYER_MAX = 2,
} physics_broadphase_layer_t;

typedef enum physics_layer {
    PHYSICS_LAYER_NON_MOVING = 0,
    PHYSICS_LAYER_MOVING = 1,
    PHYSICS_LAYER_MAX = 2,
} physics_layer_t;

typedef enum collider_shape : u8 {
    COLLIDER_SHAPE_CUBE,
    COLLIDER_SHAPE_SPHERE,
    COLLIDER_SHAPE_PLANE,
} collider_shape_t;

typedef enum physics_motion : u8 {
    PHYSICS_MOTION_STATIC,
    PHYSICS_MOTION_KINEMATIC,
    PHYSICS_MOTION_DYNAMIC,
} physics_motion_t;

typedef struct physics_body {
    u32 id;
} physics_body_t;

typedef struct veloctiy {
    vec3 value;
} velocity_t;

extern ECS_COMPONENT_DECLARE(physics_body_t);
extern ECS_COMPONENT_DECLARE(velocity_t);
