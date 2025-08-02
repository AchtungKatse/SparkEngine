#pragma once

#include "Spark/math/math_types.h"
#include "Spark/types/aabb.h"
#include "Spark/types/plane.h"

typedef struct frustum {
    plane_t far_plane;
    plane_t near_plane;
    plane_t left_plane;
    plane_t right_plane;
    plane_t top_plane;
    plane_t bottom_plane;
} frustum_t;

frustum_t frustum_create(vec3 origin, vec3 up, vec3 right, vec3 forward, float aspect, float fov_y, float near, float far);
b8 frustum_contains_point(frustum_t* frustum, vec3 point);
b8 frustum_contains_aabb(frustum_t* frustum, aabb_t aabb, vec3 offset);
