#include "Spark/renderer/ui/ui_text.h"
#include "Spark/core/sstring.h"
#include "Spark/ecs/entity.h"
#include "Spark/math/quat.h"
#include "Spark/math/vec2.h"
#include "Spark/renderer/mesh.h"
#include "Spark/renderer/renderer_frontend.h"
#include "Spark/types/transforms.h"

void text_mesh_add_vertex(f32* vertex_buffer, u32 first_vertex, vec2 pos, vec2 uv) {
    vertex_buffer[first_vertex + 0] = pos.x;
    vertex_buffer[first_vertex + 1] = pos.y;
    vertex_buffer[first_vertex + 2] = uv.x;
    vertex_buffer[first_vertex + 3] = uv.y;
}

void text_mesh_add_character(f32* vertex_buffer, u16* index_buffer, u32 vertex_index, u32 index_offset, char character, vec2 position) {
    const f32 uv_scale = 1.0f / 16.0f;
    vec2 uv_base = {
        .x = (character & 0xf) * uv_scale,
        .y = (character >> 0x4) * uv_scale,
    };

    const f32 width = 1.0f;
    const f32 height = 1.0f;

    text_mesh_add_vertex(vertex_buffer, vertex_index * 4 + 0, position, uv_base);
    text_mesh_add_vertex(vertex_buffer, vertex_index * 4 + 4,  vec2_add(position, (vec2){ .x = width, .y = 0      }), vec2_add(uv_base, (vec2){ .x = uv_scale, .y = 0 }));
    text_mesh_add_vertex(vertex_buffer, vertex_index * 4 + 8,  vec2_add(position, (vec2){ .x = width, .y = height }), vec2_add(uv_base, (vec2){ .x = uv_scale, .y = uv_scale }));
    text_mesh_add_vertex(vertex_buffer, vertex_index * 4 + 12, vec2_add(position, (vec2){ .x = 0,     .y = height }), vec2_add(uv_base, (vec2){ .x = 0,        .y = uv_scale }));

    index_buffer[index_offset + 0] = vertex_index + 0;
    index_buffer[index_offset + 1] = vertex_index + 1;
    index_buffer[index_offset + 2] = vertex_index + 2;

    index_buffer[index_offset + 3] = vertex_index + 0;
    index_buffer[index_offset + 4] = vertex_index + 2;
    index_buffer[index_offset + 5] = vertex_index + 3;
}

entity_t ui_text_create(ecs_world_t* world, const char* text) {
    f32 vertex_buffer[UI_TEXT_MAX_LENGTH * 4 * 4];
    u16 index_buffer[UI_TEXT_MAX_LENGTH * 4];
    u32 vertex_count = 0;
    u32 index_count = 0;

    u32 text_length = string_length(text);

    vec2 position = {};
    for (u32 i = 0; i < text_length && i < UI_TEXT_MAX_LENGTH; i++) {
        if (text[i] == '\n') {
            position.x = 0;
            position.y += 1;
            continue;
        }

        text_mesh_add_character(vertex_buffer, index_buffer, vertex_count, index_count, text[i], position);
        vertex_count += 4;
        index_count += 6;

        position.x += .6125f;
    }

    mesh_t mesh = renderer_create_mesh(vertex_buffer, vertex_count, sizeof(f32) * 4, index_buffer, index_count, sizeof(u16));

    entity_t e = entity_create(world);
    ENTITY_SET_COMPONENT(world, e, mesh_t, mesh);
    ENTITY_SET_COMPONENT(world, e, local_to_world_t, {});
    ENTITY_SET_COMPONENT(world, e, translation_2d_t, {});

    scale_2d_t scale = { .value = { .x = .125f, .y = .125f } };
    ENTITY_SET_COMPONENT(world, e, scale_2d_t, scale);
    ENTITY_SET_COMPONENT(world, e, rotation_t, { .value = quat_identity() });

    return e;
}
entity_t ui_text_update(const char* text);
