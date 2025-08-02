#pragma once

#include "Spark/ecs/ecs.h"
#include "Spark/math/math_types.h"
#include "Spark/types/plane.h"

typedef struct aabb {
    vec3 center;
    vec3 extents;
} aabb_t;

extern ECS_COMPONENT_DECLARE(aabb_t);

b8 aabb_intersects_plane(aabb_t* aabb, plane_t* plane, vec3 offset);
