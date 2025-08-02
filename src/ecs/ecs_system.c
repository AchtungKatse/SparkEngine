#include "Spark/ecs/ecs.h"
#include "Spark/ecs/ecs_world.h"

void ecs_system_create(struct ecs_world* world, ecs_phase_t phase, const ecs_query_create_info_t* create_info, void (*callback)(ecs_iterator_t*), const char* name) {
#if SPARK_DEBUG
    for (u32 i = 0; i < create_info->component_count; i++) {
        SASSERT(create_info->components[i] != 0, "Cannot create system, required component was not initialized (index: %d).", i);
    }
    for (u32 i = 0; i < create_info->without_component_count; i++) {
        SASSERT(create_info->without_components[i] != 0, "Cannot create system, required without_component was not initialized (index: %d).", i);
    }
#endif

    ecs_system_t system = {
        .query = ecs_query_create(world, create_info),
        .callback = callback
    };

#if SPARK_DEBUG
    system.name = name;
#endif

    darray_ecs_system_push(&world->systems[phase], system);
}

void ecs_system_destroy(ecs_system_t* system) {

}
