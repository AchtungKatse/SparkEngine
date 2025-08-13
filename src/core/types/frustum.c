#include "Spark/types/frustum.h"
#include "Spark/math/smath.h"
#include "Spark/math/vec3.h"
#include "Spark/types/aabb.h"
#include "Spark/types/plane.h"

frustum_t frustum_create(vec3 origin, vec3 up, vec3 right, vec3 forward, float aspect, float fov_y, float near, float far) {
    const f32 half_v_size = far * stan(fov_y * .5f);
    const f32 half_h_size = half_v_size * aspect;
    const vec3 front_mul_far = vec3_mul_scalar(forward, far);

    vec3 r_point = vec3_add(front_mul_far, vec3_mul_scalar(right, half_h_size));
    vec3 l_point = vec3_add(front_mul_far, vec3_mul_scalar(right, -half_h_size));
    vec3 t_point = vec3_sub(front_mul_far, vec3_mul_scalar(up, half_v_size));
    vec3 b_point = vec3_sub(front_mul_far, vec3_mul_scalar(up, half_v_size));

    vec3 f_right = vec3_cross(vec3_normalized(vec3_sub(origin, r_point)), up);
    vec3 f_left = vec3_cross(vec3_normalized(vec3_sub(origin, l_point)), up);
    vec3 f_top = vec3_cross(vec3_normalized(vec3_sub(origin, t_point)), right);
    vec3 f_bottom = vec3_cross(vec3_normalized(vec3_sub(origin, b_point)), right);

    frustum_t frustum = {
        .near_plane   = plane_create(forward, vec3_add(origin, vec3_mul_scalar(forward, near))),
        .far_plane    = plane_create(vec3_mul_scalar(forward, -1.0f), vec3_add(origin, front_mul_far)),
        .right_plane  = plane_create(f_right, origin),
        .left_plane   = plane_create(f_left, origin),
        .top_plane    = plane_create(f_top, origin),
        .bottom_plane = plane_create(f_bottom, origin),
    };

    return frustum;
}

b8 frustum_contains_point(frustum_t* frustum, vec3 point) {
    return 
        plane_distance_to_point(&frustum->far_plane,    point) >= 0 &&
        plane_distance_to_point(&frustum->near_plane,   point) >= 0 &&
        plane_distance_to_point(&frustum->right_plane,  point) >= 0 &&
        plane_distance_to_point(&frustum->left_plane,   point) >= 0 &&
        plane_distance_to_point(&frustum->top_plane,    point) >= 0 &&
        plane_distance_to_point(&frustum->bottom_plane, point) >= 0;
}

b8 frustum_contains_aabb(frustum_t* frustum, aabb_t aabb, vec3 offset) {
    return 
       aabb_intersects_plane(&aabb, &frustum->far_plane, offset) &&
        aabb_intersects_plane(&aabb, &frustum->near_plane, offset) && 
        aabb_intersects_plane(&aabb, &frustum->right_plane, offset) &&
        // aabb_intersects_plane(&aabb, &frustum->left_plane, offset) &&
        aabb_intersects_plane(&aabb, &frustum->top_plane, offset) &&
        aabb_intersects_plane(&aabb, &frustum->bottom_plane, offset);
        // (aabb_intersects_plane(&aabb, &frustum->far_plane, offset)    || plane_distance_to_point(&frustum->far_plane, center) >= 0) &&
        // (aabb_intersects_plane(&aabb, &frustum->near_plane, offset)   || plane_distance_to_point(&frustum->near_plane, center) >= 0) &&
        // (aabb_intersects_plane(&aabb, &frustum->right_plane, offset)  || plane_distance_to_point(&frustum->right_plane, center) >= 0) &&
        // (aabb_intersects_plane(&aabb, &frustum->left_plane, offset)   || plane_distance_to_point(&frustum->left_plane, center) >= 0) &&
        // (aabb_intersects_plane(&aabb, &frustum->top_plane, offset)    || plane_distance_to_point(&frustum->top_plane, center) >= 0) &&
        // (aabb_intersects_plane(&aabb, &frustum->bottom_plane, offset) || plane_distance_to_point(&frustum->bottom_plane, center) >= 0);
}
