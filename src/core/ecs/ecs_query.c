#include "Spark/containers/generic/darray_ints.h"
#include "Spark/core/smemory.h"
#include "Spark/ecs/ecs.h"
#include "Spark/ecs/ecs_world.h"
#include "Spark/ecs/entity.h"

#define ECS_QUERY_INITIAL_CAPACITY 5
darray_impl(ecs_query_t, ecs_query);

// u32 __builtin_stdc_trailing_zeros(ecs_component_id component);
s32 sort_components(void* a, void* b) {
    return *(ecs_component_id*)a < *(ecs_component_id*)b;
}

ecs_query_t* ecs_query_create(struct ecs_world* world, const ecs_query_create_info_t* create_info) {
    // Check if query already exists
    const u32 max_component_count = 128;
    u32 total_component_count = create_info->component_count + create_info->without_component_count;
    u32 all_components[max_component_count];

    scopy_memory(all_components, create_info->components, sizeof(ecs_component_id) * create_info->component_count);
    scopy_memory(all_components + create_info->component_count, create_info->without_components, sizeof(ecs_component_id) * create_info->without_component_count);

    u64 query_hash = 0x45d9f3bu;
    for (u32 i = 0; i < total_component_count; i++) {
        query_hash ^= all_components[i];
        u32 trailing_zeros = __builtin_stdc_trailing_zeros(query_hash);
        query_hash >>= trailing_zeros;
        trailing_zeros ^= 0x45d9f3bu;
    }

    // TODO: This is O(n) and slow
    for (u32 i = 0; i < world->queries.count; i++) {
        if (query_hash == world->queries.data[i].hash) {
            return &world->queries.data[i];
        }
    }

    // Create component set
    ecs_query_t query = {
        .world = world,
        .hash = query_hash,
    };

    if (create_info->component_count > 0) {
        darray_u32_create(create_info->component_count, &query.components);
        darray_u32_push_range(&query.components, create_info->component_count, create_info->components);
    }
    if (create_info->without_component_count > 0) {
        darray_u32_create(create_info->without_component_count, &query.without_components);
        darray_u32_push_range(&query.without_components, create_info->without_component_count, create_info->without_components);
    }

    // Find matching archetypes
    darray_u32_create(ECS_QUERY_INITIAL_CAPACITY, &query.archetype_indices);
    for (u32 i = 0; i < world->archetypes.count; i++) {
        entity_archetype_t* archetype = &world->archetypes.data[i];
        if (ecs_query_matches_archetype(&query, archetype)) {
            darray_u32_push(&query.archetype_indices, archetype->archetype_id);
        }
    }

    // Add query to world
    u32 query_index = world->queries.count;
    darray_ecs_query_push(&world->queries, query);
    return &world->queries.data[query_index];
}

void ecs_query_destroy(ecs_query_t* query) {
    darray_u32_destroy(&query->archetype_indices);
    if (query->components.count > 0) {
        darray_u32_destroy(&query->components);
    }
    if (query->without_components.count > 0) {
        darray_u32_destroy(&query->without_components);
    }
}

b8 ecs_query_matches_archetype(ecs_query_t* query, entity_archetype_t* archetype) {
    if (query->components.count == 0) {
        SWARN("Creating query with no components to match");
        // return false;
    }
    // Query cannot match archetype if the archetype does not have all required components
    // This is mostly just here to avoid matching empty archetypes
    if (archetype->component_set.count < query->components.count ) {
        return false;
    }

    for (u32 i = 0; i < query->without_components.count; i++) {
        if (ecs_component_set_contains(&archetype->component_set, query->without_components.data[i])) {
            return false;
        }
    }

    for (u32 i = 0; i < query->components.count; i++) {
        if (!ecs_component_set_contains(&archetype->component_set, query->components.data[i])) {
            return false;
        }
    }

    if (archetype->archetype_id == 0) {
        SERROR("How are we matching the default archetype?");
    }
    return true;
}

void ecs_query_create_iterator(ecs_query_t* query, ecs_iterator_t* out_iterator) {

}

void ecs_query_iterate(ecs_query_t* query, void (iterate_function)(ecs_iterator_t* iterator)) {
    // Create iterator
    void* component_arrays[MAX_QUERY_COMPONENT_COUNT];

    ecs_iterator_t iterator = {
        .component_data = component_arrays,
        .component_count = query->components.count,
        .world = query->world,
    };

    for (u32 i = 0; i < query->archetype_indices.count; i++) {
        entity_archetype_t* archetype = &query->world->archetypes.data[query->archetype_indices.data[i]];
        iterator.archetype = archetype;
        if (archetype->entities.count <= 0) {
            continue;
        }

        // Set component arrays
        SASSERT(query->components.count < MAX_QUERY_COMPONENT_COUNT, "QUERY HAS TOO MANY COMPONENTS");
        for (u32 j = 0; j < query->components.count; j++) {
            ecs_component_id component = query->components.data[j];

            u32 component_index = ecs_component_set_get_index(&archetype->component_set, component);
            if (component_index == INVALID_ID) {
                SERROR("Should not get invalid ID from archetype that matches query.");
                continue;
            }
            component_arrays[j] = archetype->columns.data[component_index].data;
            SASSERT(archetype->columns.data[component_index].component_stride == query->world->components.data[component].stride, "Failed to get correct component from query.");
        }

        iterator.entity_count = archetype->entities.count;

        // Call function
        iterate_function(&iterator);
    }
}
