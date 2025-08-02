#include "Spark/renderer/ui/ui_image.h"
#include "Spark/ecs/entity.h"
#include "Spark/math/quat.h"
#include "Spark/renderer/mesh.h"
#include "Spark/renderer/renderer_frontend.h"
#include "Spark/types/transforms.h"

entity_t ui_image_create(ecs_world_t* world, ui_rect_t rect) {
    // Create the mesh
    f32 verts[] = {
        0.0f, 0.0f, 0.0f, 0.0f,
        rect.extents.x, 0.0f, 1.0f, 0.0f,
        rect.extents.x, rect.extents.y, 1.0f, 1.0f,
        0.0f, rect.extents.y, 0.0f, 1.0f,
    };

    u16 indices[] = {
        0, 1, 2,
        0, 2, 3,
    };

    mesh_t mesh = renderer_create_mesh(verts, 4, sizeof(f32) * 4, indices, 6, sizeof(u16));
    entity_t image = entity_create(world);

    ENTITY_SET_COMPONENT(world, image, mesh_t, mesh);

    translation_2d_t translation = { 
        .value = {
            .x = rect.offset.x,
            .y = rect.offset.y,
        }
    };
    scale_2d_t scale = {
        .value = {
            .x = 1,
            .y = 1,
        },
    };

    ENTITY_SET_COMPONENT(world, image, local_to_world_t, {});
    ENTITY_SET_COMPONENT(world, image, translation_2d_t, translation);
    ENTITY_SET_COMPONENT(world, image, scale_2d_t, scale);
    ENTITY_SET_COMPONENT(world, image, rotation_t, { .value = quat_identity() });

    return image;
}

void ui_image_update(ecs_world_t* world, entity_t entity, ui_rect_t rect);
