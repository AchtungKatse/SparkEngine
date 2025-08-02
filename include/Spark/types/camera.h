#pragma once

#include "Spark/ecs/ecs.h"

typedef struct camera {
    f32 fov;
    f32 near_clip;
    f32 far_clip;
} camera_t;

extern ECS_COMPONENT_DECLARE(camera_t);
