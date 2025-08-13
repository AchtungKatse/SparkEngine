#include "Spark/core/smemory.h"
#include "Spark/defines.h"
#include "Spark/ecs/ecs.h"
#include "Spark/math/smath.h"

void ecs_component_column_create(u32 initial_count, u32 component_stride, ecs_column_t* out_column) {
    out_column->data = sallocate(initial_count * component_stride, MEMORY_TAG_ECS);
    out_column->count = 0;
    out_column->capacity = initial_count;
    out_column->component_stride = component_stride;
}

void ecs_component_column_destroy(ecs_column_t* column) {
    if (!column->data) {
        SWARN("Trying to free null column data");
        return;
    }

    sfree(column->data, column->capacity * column->component_stride, MEMORY_TAG_ECS);
    szero_memory(column, sizeof(ecs_column_t));
}

void ecs_component_column_resize(ecs_column_t* column, u32 size) { 
    if (!column->data && size > 0) {
        column->data = sallocate(column->component_stride * size, MEMORY_TAG_ECS);
        return;
    }

    if (size > column->capacity) {
        SASSERT(column->component_stride > 0, "Resizing component column with component size of 0 is not allowed.");
        SASSERT(column->count >= 0 && column->count != INVALID_ID_U64, "Cannot resize column with negative number of elements: %d", column->count);
        void* temp = sallocate(column->component_stride * size, MEMORY_TAG_ECS);

        // Zero out new memory
        sset_memory(temp + column->count * column->component_stride, 0, (size - column->count) * column->component_stride);

        // Copy old memory
        scopy_memory(temp, column->data, column->count * column->component_stride);
        sfree(column->data, column->capacity * column->component_stride, MEMORY_TAG_ECS);

        column->data = temp;
        column->capacity = size;
    }
}

void ecs_component_column_push(ecs_column_t* column, void* data) {
    if (column->count >= column->capacity) {
        ecs_component_column_resize(column, smax(column->capacity, 1) * 2);
    }

    SASSERT(column->data, "Cannot push to empty ecs column");
    SASSERT(column, "Cannot push to null ecs column");

    scopy_memory(column->data + column->component_stride * column->count, data, column->component_stride);
    column->count++;
}

void ecs_component_column_pop(ecs_column_t* column, ecs_index row) {
    SASSERT(column->count != 0, "Cannot pop element from empty ecs_column.");
    // Copy last element to pop location
    // if (row != column->count - 1) {
        scopy_memory(column->data + column->component_stride * row, column->data + column->component_stride * (column->count - 1), column->component_stride);
    // }

    column->count--;
}
