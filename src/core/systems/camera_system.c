
#include "Spark/systems/core_systems.h"
#include "Spark/ecs/ecs.h"
#include "Spark/ecs/ecs_world.h"
#include "Spark/types/camera.h"
#include "Spark/types/transforms.h"

void update_cameras(ecs_iterator_t* iterator);

void camera_systems_initialize(ecs_world_t* world) {
    //
    // ecs_component_id components[] = {
    //     ECS_COMPONENT_ID(camera_t),
    //     ECS_COMPONENT_ID(local_to_world_t),
    // };
    //
    // ecs_system_create(world, ECS_PHASE_TRANSFORM, 2, components, update_cameras, "Update camera system");
}

void update_cameras(ecs_iterator_t* iterator) {
    // camera_t* cameras        = ECS_ITERATOR_GET_COMPONENTS(iterator, 0);
    // local_to_world_t* locals = ECS_ITERATOR_GET_COMPONENTS(iterator, 1);
    //
    // for (u32 i = 0; i < iterator->entity_count; i++) {
    //     cameras[i].view_matrix = mat4_inverse(locals[i].value);
    // }
}
