
#include "Spark/types/aabb.h"
#include "Spark/math/smath.h"
#include "Spark/math/vec3.h"

b8 aabb_intersects_plane(aabb_t* aabb, plane_t* plane, vec3 offset) {
    float r = 
        aabb->extents.x * sabs(plane->normal.x) +
        aabb->extents.y * sabs(plane->normal.y) + 
        aabb->extents.z * sabs(plane->normal.z);

    vec3 pos = vec3_add(offset, aabb->center);
    float s = plane_distance_to_point(plane, pos);
    return -r <= s;
}
