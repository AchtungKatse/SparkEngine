#include "Spark/containers/darray.h"
#include "Spark/ecs/ecs.h"
#include "Spark/ecs/ecs_world.h"
#include "Spark/ecs/entity.h"
#include "Spark/math/mat4.h"
#include "Spark/math/vec3.h"
#include "Spark/renderer/renderer_frontend.h"
#include "Spark/renderer/renderer_types.h"
#include "Spark/resources/loaders/shader_loader.h"
#include "Spark/systems/core_systems.h"
#include "Spark/types/camera.h"
#include "Spark/types/frustum.h"
#include "Spark/types/transforms.h"
#include <stdlib.h>

// Private functions
void render_cameras(ecs_iterator_t* iterator);
void render_entities(ecs_iterator_t* iterator);

// Private Types
darray_type(geometry_render_data_t, geometry_render_data);
darray_impl(geometry_render_data_t, geometry_render_data);

typedef struct render_system_state {
    ecs_query_t* render_entities_query;
    darray_geometry_render_data_t render_data;
    vec3 camera_pos;
    vec3 camera_forward;
    frustum_t view_frustum;
} render_system_state_t;

// State
render_system_state_t render_state;

// Function Impl
void render_system_initialize(ecs_world_t* world) {

    darray_geometry_render_data_create(300, &render_state.render_data);
    const ecs_component_id render_entities_components[] = { 
        ECS_COMPONENT_ID(mesh_t), 
        ECS_COMPONENT_ID(local_to_world_t), 
        ECS_COMPONENT_ID(aabb_t), 
        ECS_COMPONENT_ID(material_t)
    };

    const ecs_query_create_info_t render_entities_create_info = {
        .component_count = 4,
        .components = render_entities_components,
    };

    render_state.render_entities_query = ecs_query_create(world, &render_entities_create_info);

    const ecs_component_id render_cameras_components[] = {
        ECS_COMPONENT_ID(camera_t), 
        ECS_COMPONENT_ID(local_to_world_t),
        ECS_COMPONENT_ID(translation_t),
    };

    const ecs_query_create_info_t render_cameras_create_info = {
        .component_count = 3,
        .components = render_cameras_components,
    };

    ecs_system_create(world, ECS_PHASE_RENDER, &render_cameras_create_info, render_cameras, "render_cameras");
}

void render_system_shutdown() {
    darray_geometry_render_data_destroy(&render_state.render_data);
}

b32 sort_geometry(const void* a, const void* b) {
    const geometry_render_data_t* g1 = a;
    const geometry_render_data_t* g2 = b;

    // shader_t* s1 = shader_loader_get_shader(g1->material->shader_index);
    // shader_t* s2 = shader_loader_get_shader(g2->material->shader_index);
    //
    // // First sort by type
    // if (s1->type < s2->type) {
    //     return -1;
    // } else if (s1->type > s2->type) {
    //     return 1;
    // }
    //
    // // Then sort by material
    // if (g1->material < g2->material) {
    //     return -1;
    // } else if (g1->material > g2->material) {
    //     return 1;
    // }

    if (g1->mesh.internal_index < g2->mesh.internal_index) {
        return -1;
    } else if (g1->mesh.internal_index > g2->mesh.internal_index) {
        return 1;
    }

    // Then sort by distance
    // f32 distance_1 = vec3_distance_sqr(render_state.camera_pos, g1->position);
    // f32 distance_2 = vec3_distance_sqr(render_state.camera_pos, g2->position);
    // if (distance_1 < distance_2) {
    //     return 1;
    // } else if (distance_1 > distance_2) {
    //     return -1;
    // }
    return 0;
}

void render_cameras(ecs_iterator_t* iterator) {
    camera_t* cameras = ECS_ITERATOR_GET_COMPONENTS(iterator, 0);
    local_to_world_t* locals = ECS_ITERATOR_GET_COMPONENTS(iterator, 1);
    // translation_t* translations = ECS_ITERATOR_GET_COMPONENTS(iterator, 2);

    // Render per-camera
    vec2 screen_size = renderer_get_screen_size();
    for (u32 i = 0; i < iterator->entity_count; i++) {
        render_packet_t packet = {
            .delta_time = 0,
            .view_matrix = mat4_inverse(locals[i].value),
            .projection_matrix = mat4_projection_matrix(cameras[i].fov, cameras[i].far_clip, cameras[i].near_clip, screen_size.x, screen_size.y),
        };

        vec3 camera_up = mat4_up(locals[i].value);
        vec3 camera_right = mat4_right(locals[i].value);
        vec3 camera_forward = mat4_forward(locals[i].value);
        vec3 camera_pos = {
            {locals[i].value.data[12],
                locals[i].value.data[13],
                locals[i].value.data[14]},
        };

        render_state.view_frustum = frustum_create(camera_pos, camera_up, camera_right, camera_forward, screen_size.x / screen_size.y, cameras[i].fov, cameras[i].near_clip, cameras[i].far_clip);

        render_state.camera_pos = camera_pos;
        render_state.camera_forward = camera_forward;

        darray_geometry_render_data_clear(&render_state.render_data);
        ecs_query_iterate(render_state.render_entities_query, render_entities);

        packet.geometry_count = render_state.render_data.count;
        packet.geometries = render_state.render_data.data;


        // qsort(packet.geometries, packet.geometry_count, sizeof(geometry_render_data_t), sort_geometry);

        // for (u32 e = 0; e < packet.geometry_count; e++) {
        //     f32 distance = vec3_distance_sqr(camera_pos, packet.geometries[e].position);
        //     SDEBUG("Rendering entity %d at distance %f", e, distance);
        // }

        if (packet.geometry_count > 0) {
            renderer_draw_frame(&packet);
        }
        darray_geometry_render_data_clear(&render_state.render_data);
    }
}

void render_entities(ecs_iterator_t* iterator) {
    // Render each entity in the vameras view
    mesh_t* meshes                 = ECS_ITERATOR_GET_COMPONENTS(iterator, 0);
    local_to_world_t* locals       = ECS_ITERATOR_GET_COMPONENTS(iterator, 1);
    aabb_t* bounds                 = ECS_ITERATOR_GET_COMPONENTS(iterator, 2);
    material_t* materials          = ECS_ITERATOR_GET_COMPONENTS(iterator, 3);

    for (u32 i = 0; i < iterator->entity_count; i++) {
        vec3 geometry_pos = {
            {locals[i].value.data[12],
                locals[i].value.data[13],
                locals[i].value.data[14]},
        };
        if (!frustum_contains_aabb(&render_state.view_frustum, bounds[i], geometry_pos) &&
                vec3_distance_sqr(geometry_pos, render_state.camera_pos) > 30 * 30) {
            // continue;
        }

        geometry_render_data_t render_data = {
            .mesh = meshes[i],
            .model = locals[i].value,
            .material = &materials[i],
            .position = (vec3) {{
                locals[i].value.data[12],
                locals[i].value.data[13],
                locals[i].value.data[14],
            }},
        };


        darray_geometry_render_data_push(&render_state.render_data, render_data);
    }
}
