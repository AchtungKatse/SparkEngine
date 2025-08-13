#include "Spark/types/plane.h"
#include "Spark/math/vec3.h"

plane_t plane_create(vec3 normal, vec3 point) {
    float distance = normal.x * point.x + normal.y * point.y + normal.z * point.z;
    return (plane_t) {
        .normal = normal,
            .distance = distance,
    };
}

f32 plane_distance_to_point(plane_t* plane, vec3 point) {
    return vec3_dot(plane->normal, point) - plane->distance;
}

