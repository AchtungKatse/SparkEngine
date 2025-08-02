#pragma once

#include "Spark/math/math_types.h"

typedef struct plane {
    vec3 normal;
    f32 distance;
} plane_t;

plane_t plane_create(vec3 normal, vec3 point);
f32 plane_distance_to_point(plane_t* plane, vec3 point);
