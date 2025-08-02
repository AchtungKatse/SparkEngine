#pragma once

#include "Spark/ecs/entity.h"
#include "Spark/renderer/ui/ui_rect.h"
typedef struct ui_image {
} ui_image_t;

entity_t ui_image_create(ecs_world_t* world, ui_rect_t rect);
void ui_image_update(ecs_world_t* world, entity_t entity, ui_rect_t rect);
